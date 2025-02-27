#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <QDebug>
#include <QScrollBar>

#include "selfdrive/ui/qt/network/networking.h"

#include "common/params.h"
#include "common/watchdog.h"
#include "common/util.h"
#include "system/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"

#include "selfdrive/frogpilot/navigation/ui/navigation_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/control_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/vehicle_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/visual_settings.h"

TogglesPanel::TogglesPanel(SettingsWindow *parent) : ListWidget(parent) {
  // param, title, desc, icon
  std::vector<std::tuple<QString, QString, QString, QString>> toggle_defs{
    {
      "OpenpilotEnabledToggle",
      tr("Enable openpilot"),
      tr("Use the openpilot system for adaptive cruise control and lane keep driver assistance. Your attention is required at all times to use this feature. Changing this setting takes effect when the car is powered off."),
      "../assets/offroad/icon_openpilot.png",
    },
    {
      "ExperimentalLongitudinalEnabled",
      tr("openpilot Longitudinal Control (Alpha)"),
      QString("<b>%1</b><br><br>%2")
      .arg(tr("WARNING: openpilot longitudinal control is in alpha for this car and will disable Automatic Emergency Braking (AEB)."))
      .arg(tr("On this car, openpilot defaults to the car's built-in ACC instead of openpilot's longitudinal control. "
              "Enable this to switch to openpilot longitudinal control. Enabling Experimental mode is recommended when enabling openpilot longitudinal control alpha.")),
      "../assets/offroad/icon_speed_limit.png",
    },
    {
      "ExperimentalMode",
      tr("Experimental Mode"),
      "",
      "../assets/img_experimental_white.svg",
    },
    {
      "DisengageOnAccelerator",
      tr("Disengage on Accelerator Pedal"),
      tr("When enabled, pressing the accelerator pedal will disengage openpilot."),
      "../assets/offroad/icon_disengage_on_accelerator.svg",
    },
    {
      "IsLdwEnabled",
      tr("Enable Lane Departure Warnings"),
      tr("Receive alerts to steer back into the lane when your vehicle drifts over a detected lane line without a turn signal activated while driving over 31 mph (50 km/h)."),
      "../assets/offroad/icon_warning.png",
    },
    {
      "RecordFront",
      tr("Record and Upload Driver Camera"),
      tr("Upload data from the driver facing camera and help improve the driver monitoring algorithm."),
      "../assets/offroad/icon_monitoring.png",
    },
    {
      "IsMetric",
      tr("Use Metric System"),
      tr("Display speed in km/h instead of mph."),
      "../assets/offroad/icon_metric.png",
    },
#ifdef ENABLE_MAPS
    {
      "NavSettingTime24h",
      tr("Show ETA in 24h Format"),
      tr("Use 24h format instead of am/pm"),
      "../assets/offroad/icon_metric.png",
    },
    {
      "NavSettingLeftSide",
      tr("Show Map on Left Side of UI"),
      tr("Show map on left side when in split screen view."),
      "../assets/offroad/icon_road.png",
    },
#endif
  };


  std::vector<QString> longi_button_texts{tr("Aggressive"), tr("Standard"), tr("Relaxed")};
  long_personality_setting = new ButtonParamControl("LongitudinalPersonality", tr("Driving Personality"),
                                          tr("Standard is recommended. In aggressive mode, openpilot will follow lead cars closer and be more aggressive with the gas and brake. "
                                             "In relaxed mode openpilot will stay further away from lead cars. On supported cars, you can cycle through these personalities with "
                                             "your steering wheel distance button."),
                                          "../assets/offroad/icon_speed_limit.png",
                                          longi_button_texts);

  // set up uiState update for personality setting
  QObject::connect(uiState(), &UIState::uiUpdate, this, &TogglesPanel::updateState);

  for (auto &[param, title, desc, icon] : toggle_defs) {
    auto toggle = new ParamControl(param, title, desc, icon, this);

    bool locked = params.getBool((param + "Lock").toStdString());
    toggle->setEnabled(!locked);

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    // insert longitudinal personality after NDOG toggle
    if (param == "DisengageOnAccelerator") {
      addItem(long_personality_setting);
    }
  }

  // Toggles with confirmation dialogs
  toggles["ExperimentalMode"]->setActiveIcon("../assets/img_experimental.svg");
  toggles["ExperimentalMode"]->setConfirmation(true, true);
  toggles["ExperimentalLongitudinalEnabled"]->setConfirmation(true, false);

  connect(toggles["ExperimentalLongitudinalEnabled"], &ToggleControl::toggleFlipped, [=]() {
    updateToggles();
  });

  connect(toggles["IsMetric"], &ToggleControl::toggleFlipped, [=]() {
    updateMetric();
  });
}

void TogglesPanel::updateState(const UIState &s) {
  const SubMaster &sm = *(s.sm);

  if (sm.updated("controlsState")) {
    auto personality = sm["controlsState"].getControlsState().getPersonality();
    if (personality != s.scene.personality && s.scene.started && isVisible()) {
      long_personality_setting->setCheckedButton(static_cast<int>(personality));
    }
    uiState()->scene.personality = personality;
  }
}

void TogglesPanel::expandToggleDescription(const QString &param) {
  toggles[param.toStdString()]->showDescription();
}

void TogglesPanel::showEvent(QShowEvent *event) {
  updateToggles();
}

void TogglesPanel::updateToggles() {
  auto disengage_on_accelerator_toggle = toggles["DisengageOnAccelerator"];
  disengage_on_accelerator_toggle->setVisible(!params.getBool("AlwaysOnLateral"));
  auto driver_camera_toggle = toggles["RecordFront"];
  driver_camera_toggle->setVisible(!(params.getBool("DeviceManagement") && params.getBool("NoLogging") && params.getBool("NoUploads")));
  auto nav_settings_left_toggle = toggles["NavSettingLeftSide"];
  nav_settings_left_toggle->setVisible(!params.getBool("FullMap"));

  auto experimental_mode_toggle = toggles["ExperimentalMode"];
  auto op_long_toggle = toggles["ExperimentalLongitudinalEnabled"];
  const QString e2e_description = QString("%1<br>"
                                          "<h4>%2</h4><br>"
                                          "%3<br>"
                                          "<h4>%4</h4><br>"
                                          "%5<br>"
                                          "<h4>%6</h4><br>"
                                          "%7")
                                  .arg(tr("openpilot defaults to driving in <b>chill mode</b>. Experimental mode enables <b>alpha-level features</b> that aren't ready for chill mode. Experimental features are listed below:"))
                                  .arg(tr("End-to-End Longitudinal Control"))
                                  .arg(tr("Let the driving model control the gas and brakes. openpilot will drive as it thinks a human would, including stopping for red lights and stop signs. "
                                          "Since the driving model decides the speed to drive, the set speed will only act as an upper bound. This is an alpha quality feature; "
                                          "mistakes should be expected."))
                                  .arg(tr("Navigate on openpilot"))
                                  .arg(tr("When navigation has a destination, openpilot will input the map information into the model. This provides useful context for the model and allows openpilot to keep left or right "
                                          "appropriately at forks/exits. Lane change behavior is unchanged and still activated by the driver. This is an alpha quality feature; mistakes should be expected, particularly around "
                                          "exits and forks. These mistakes can include unintended laneline crossings, late exit taking, driving towards dividing barriers in the gore areas, etc."))
                                  .arg(tr("New Driving Visualization"))
                                  .arg(tr("The driving visualization will transition to the road-facing wide-angle camera at low speeds to better show some turns. The Experimental mode logo will also be shown in the top right corner. "
                                          "When a navigation destination is set and the driving model is using it as input, the driving path on the map will turn green."));

  const bool is_release = params.getBool("IsReleaseBranch");
  auto cp_bytes = params.get("CarParamsPersistent");
  if (!cp_bytes.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(cp_bytes.data(), cp_bytes.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();

    if (!CP.getExperimentalLongitudinalAvailable() || is_release) {
      params.remove("ExperimentalLongitudinalEnabled");
    }
    op_long_toggle->setVisible(CP.getExperimentalLongitudinalAvailable() && !is_release);
    if (hasLongitudinalControl(CP)) {
      // normal description and toggle
      bool conditional_experimental = params.getBool("ConditionalExperimental");
      if (conditional_experimental) {
        params.putBool("ExperimentalMode", true);
        params.putBool("ExperimentalModeConfirmed", true);
        experimental_mode_toggle->refresh();
      }
      experimental_mode_toggle->setEnabled(!conditional_experimental);
      experimental_mode_toggle->setDescription(e2e_description);
      long_personality_setting->setEnabled(true);
    } else {
      // no long for now
      experimental_mode_toggle->setEnabled(false);
      long_personality_setting->setEnabled(false);
      params.remove("ExperimentalMode");

      const QString unavailable = tr("Experimental mode is currently unavailable on this car since the car's stock ACC is used for longitudinal control.");

      QString long_desc = unavailable + " " + \
                          tr("openpilot longitudinal control may come in a future update.");
      if (CP.getExperimentalLongitudinalAvailable()) {
        if (is_release) {
          long_desc = unavailable + " " + tr("An alpha version of openpilot longitudinal control can be tested, along with Experimental mode, on non-release branches.");
        } else {
          long_desc = tr("Enable the openpilot longitudinal control (alpha) toggle to allow Experimental mode.");
        }
      }
      experimental_mode_toggle->setDescription("<b>" + long_desc + "</b><br><br>" + e2e_description);
    }

    experimental_mode_toggle->refresh();
  } else {
    experimental_mode_toggle->setDescription(e2e_description);
    op_long_toggle->setVisible(false);
  }
}

DevicePanel::DevicePanel(SettingsWindow *parent) : ListWidget(parent) {
  setSpacing(50);
  addItem(new LabelControl(tr("Dongle ID"), getDongleId().value_or(tr("N/A"))));
  addItem(new LabelControl(tr("Serial"), params.get("HardwareSerial").c_str()));

  // offroad-only buttons

  auto dcamBtn = new ButtonControl(tr("Driver Camera"), tr("PREVIEW"),
                                   tr("Preview the driver facing camera to ensure that driver monitoring has good visibility. (vehicle must be off)"));
  connect(dcamBtn, &ButtonControl::clicked, [=]() { emit showDriverView(); });
  addItem(dcamBtn);

  auto resetCalibBtn = new ButtonControl(tr("Reset Calibration"), tr("RESET"), "");
  connect(resetCalibBtn, &ButtonControl::showDescriptionEvent, this, &DevicePanel::updateCalibDescription);
  connect(resetCalibBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to reset calibration?"), tr("Reset"), this)) {
      params.remove("CalibrationParams");
      params.remove("LiveTorqueParameters");
    }
  });
  addItem(resetCalibBtn);

  auto retrainingBtn = new ButtonControl(tr("Review Training Guide"), tr("REVIEW"), tr("Review the rules, features, and limitations of openpilot"));
  connect(retrainingBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to review the training guide?"), tr("Review"), this)) {
      emit reviewTrainingGuide();
    }
  });
  addItem(retrainingBtn);

  if (Hardware::TICI()) {
    auto regulatoryBtn = new ButtonControl(tr("Regulatory"), tr("VIEW"), "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file("../assets/offroad/fcc.html");
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    });
    addItem(regulatoryBtn);
  }

  auto translateBtn = new ButtonControl(tr("Change Language"), tr("CHANGE"), "");
  connect(translateBtn, &ButtonControl::clicked, [=]() {
    QMap<QString, QString> langs = getSupportedLanguages();
    QString selection = MultiOptionDialog::getSelection(tr("Select a language"), langs.keys(), langs.key(uiState()->language), this);
    if (!selection.isEmpty()) {
      // put language setting, exit Qt UI, and trigger fast restart
      params.put("LanguageSetting", langs[selection].toStdString());
      qApp->exit(18);
      watchdog_kick(0);
    }
  });
  addItem(translateBtn);

  // Backup FrogPilot
  std::vector<QString> frogpilotBackupOptions{tr("저장"), tr("삭제"), tr("복구")};
  FrogPilotButtonsControl *frogpilotBackup = new FrogPilotButtonsControl(tr("FrogPilot 백업"), tr("FrogPilot 백업, 삭제 또는 복원합니다."), "", frogpilotBackupOptions);

  connect(frogpilotBackup, &FrogPilotButtonsControl::buttonClicked, [=](int id) {
    QDir backupDir("/data/backups");

    if (id == 0) {
      QString nameSelection = InputDialog::getText(tr("백업 이름"), this, "", false, 1);
      if (!nameSelection.isEmpty()) {
        std::thread([=]() {
          frogpilotBackup->setValue(tr("백업 중..."));

          std::string fullBackupPath = backupDir.absolutePath().toStdString() + "/" + nameSelection.toStdString();

          std::ostringstream commandStream;
          commandStream << "mkdir -p " << std::quoted(fullBackupPath)
                        << " && rsync -av /data/openpilot/ " << std::quoted(fullBackupPath + "/");
          std::string command = commandStream.str();

          int result = std::system(command.c_str());
          if (result == 0) {
            std::cout << "Backup successful to " << fullBackupPath << std::endl;
            frogpilotBackup->setValue(tr("성공!"));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            frogpilotBackup->setValue("");
          } else {
            std::cerr << "Backup failed with error code: " << result << std::endl;
            frogpilotBackup->setValue(tr("실패..."));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            frogpilotBackup->setValue("");
          }
        }).detach();
      }
    } else if (id == 1) {
      QStringList backupNames = backupDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

      QString selection = MultiOptionDialog::getSelection(tr("삭제할 백업을 선택"), backupNames, "", this);
      if (!selection.isEmpty()) {
        if (!ConfirmationDialog::confirm(tr("이 백업을 삭제하시겠습니까?"), tr("삭제"), this)) return;
        std::thread([=]() {
          frogpilotBackup->setValue(tr("삭제중..."));
          QDir dirToDelete(backupDir.absoluteFilePath(selection));
          if (dirToDelete.removeRecursively()) {
            frogpilotBackup->setValue(tr("삭제!"));
          } else {
            frogpilotBackup->setValue(tr("실패..."));
          }
          std::this_thread::sleep_for(std::chrono::seconds(3));
          frogpilotBackup->setValue("");
        }).detach();
      }
    } else {
      QStringList backupNames = backupDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

      QString selection = MultiOptionDialog::getSelection(tr("복구 포인트를 선택하세요"), backupNames, "", this);
      if (!selection.isEmpty()) {
        if (!ConfirmationDialog::confirm(tr("FrogPilot의 이 버전을 복원하시겠습니까"), tr("복구"), this)) return;
        std::thread([=]() {
          frogpilotBackup->setValue(tr("Restoring..."));

          std::string sourcePath = backupDir.absolutePath().toStdString() + "/" + selection.toStdString();
          std::string targetPath = "/data/safe_staging/finalized";
          std::string consistentFilePath = targetPath + "/.overlay_consistent";

          std::ostringstream commandStream;
          commandStream << "rsync -av --delete --exclude='.overlay_consistent' " << std::quoted(sourcePath + "/") << " " << std::quoted(targetPath + "/");
          std::string command = commandStream.str();
          int result = std::system(command.c_str());

          if (result == 0) {
            std::cout << "Restore successful from " << sourcePath << " to " << targetPath << std::endl;
            std::ofstream consistentFile(consistentFilePath);
            if (consistentFile) {
              consistentFile.close();
              std::cout << ".overlay_consistent file created successfully." << std::endl;
            } else {
              std::cerr << "Failed to create .overlay_consistent file." << std::endl;
              frogpilotBackup->setValue(tr("실패..."));
              std::this_thread::sleep_for(std::chrono::seconds(3));
              frogpilotBackup->setValue("");
            }
            Hardware::reboot();
          } else {
            std::cerr << "Restore failed with error code: " << result << std::endl;
          }
        }).detach();
      }
    }
  });
  addItem(frogpilotBackup);

  // Backup toggles
  std::vector<QString> toggleBackupOptions{tr("저장"), tr("삭제"), tr("복구")};
  FrogPilotButtonsControl *toggleBackup = new FrogPilotButtonsControl(tr("토글 백업"), tr("토글을 백업, 삭제 또는 복원합니다.."), "", toggleBackupOptions);

  connect(toggleBackup, &FrogPilotButtonsControl::buttonClicked, [=](int id) {
    QDir backupDir("/data/toggle_backups");

    if (id == 0) {
      QString nameSelection = InputDialog::getText(tr("백업 이름 지정"), this, "", false, 1);
      if (!nameSelection.isEmpty()) {
        std::thread([=]() {
          toggleBackup->setValue(tr("저장 중..."));

          std::string fullBackupPath = backupDir.absolutePath().toStdString() + "/" + nameSelection.toStdString() + "/";

          std::ostringstream commandStream;
          commandStream << "mkdir -p " << std::quoted(fullBackupPath)
                        << " && rsync -av /data/params/d/ " << std::quoted(fullBackupPath);
          std::string command = commandStream.str();

          int result = std::system(command.c_str());
          if (result == 0) {
            std::cout << "Backup successful to " << fullBackupPath << std::endl;
            toggleBackup->setValue(tr("성공!"));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            toggleBackup->setValue("");
          } else {
            std::cerr << "Backup failed with error code: " << result << std::endl;
            toggleBackup->setValue(tr("실패..."));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            toggleBackup->setValue("");
          }
        }).detach();
      }
    } else if (id == 1) {
      QStringList backupNames = backupDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

      QString selection = MultiOptionDialog::getSelection(tr("Select a backup to delete"), backupNames, "", this);
      if (!selection.isEmpty()) {
        if (!ConfirmationDialog::confirm(tr("이 백업을 삭제하시겠습니까?"), tr("삭제"), this)) return;
        std::thread([=]() {
          toggleBackup->setValue(tr("삭제 중..."));
          QDir dirToDelete(backupDir.absoluteFilePath(selection));
          if (dirToDelete.removeRecursively()) {
            toggleBackup->setValue(tr("삭제!"));
          } else {
            toggleBackup->setValue(tr("실패..."));
          }
          std::this_thread::sleep_for(std::chrono::seconds(3));
          toggleBackup->setValue("");
        }).detach();
      }
    } else {
      QStringList backupNames = backupDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

      QString selection = MultiOptionDialog::getSelection(tr("Select a restore point"), backupNames, "", this);
      if (!selection.isEmpty()) {
        if (!ConfirmationDialog::confirm(tr("이 토글 백업을 복원하시겠습니까?"), tr("복구"), this)) return;
        std::thread([=]() {
          toggleBackup->setValue(tr("복구중..."));

          std::string sourcePath = backupDir.absolutePath().toStdString() + "/" + selection.toStdString() + "/";
          std::string targetPath = "/data/params/d/";

          std::ostringstream commandStream;
          commandStream << "rsync -av --delete " << std::quoted(sourcePath) << " " << std::quoted(targetPath);
          std::string command = commandStream.str();

          int result = std::system(command.c_str());

          if (result == 0) {
            std::cout << "Restore successful from " << sourcePath << " to " << targetPath << std::endl;
            toggleBackup->setValue(tr("성공!"));
            updateFrogPilotToggles();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            toggleBackup->setValue("");
          } else {
            std::cerr << "Restore failed with error code: " << result << std::endl;
            toggleBackup->setValue(tr("실패..."));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            toggleBackup->setValue("");
          }
        }).detach();
      }
    }
  });
  addItem(toggleBackup);

  // Delete driving footage button
  auto deleteDrivingDataBtn = new ButtonControl(tr("주행기록 삭제"), tr("삭제"), tr("이 버튼을 사용하면 모든 항목을 영구적으로 빠르고 안전하게 삭제할 수 있습니다."
  "장치에서 저장된 주행 영상과 데이터. 개인 정보 보호 또는 공간 확보에 이상적입니다")
  );
  connect(deleteDrivingDataBtn, &ButtonControl::clicked, [=]() {
    if (!ConfirmationDialog::confirm(tr("운전 영상과 데이터를 모두 영구적으로 삭제하시겠습니까?"), tr("삭제"), this)) return;
    std::thread([&] {
      deleteDrivingDataBtn->setValue(tr("영상을 삭제하는 중..."));
      std::system("rm -rf /data/media/0/realdata");
      deleteDrivingDataBtn->setValue(tr("삭제 완료!"));
      std::this_thread::sleep_for(std::chrono::seconds(3));
      deleteDrivingDataBtn->setValue("");
    }).detach();
  });
  addItem(deleteDrivingDataBtn);

  // Delete long term toggle storage button
  auto resetTogglesBtn = new ButtonControl(tr("토글을 기본값으로 재설정"), tr("리셋"), tr("토글 설정을 기본 설정으로 재설정합니다."));
  connect(resetTogglesBtn, &ButtonControl::clicked, [=]() {
    if (!ConfirmationDialog::confirm(tr("모든 토글 설정을 완전히 재설정하시겠습니까?"), tr("리셋"), this)) return;
    std::thread([&] {
      resetTogglesBtn->setValue(tr("토글 재설정 중..."));
      std::system("rm -rf /persist/params");
      params.putBool("DoToggleReset", true);
      resetTogglesBtn->setValue(tr("리셋!"));
      std::this_thread::sleep_for(std::chrono::seconds(2));
      resetTogglesBtn->setValue(tr("재부팅 중..."));
      std::this_thread::sleep_for(std::chrono::seconds(3));
      Hardware::reboot();
    }).detach();
  });
  addItem(resetTogglesBtn);

  // Panda flashing button
  auto flashPandaBtn = new ButtonControl(tr("판다 플래시"), tr("플래시"), tr("이 버튼을 사용하여 Panda 장치의 펌웨어 문제를 해결하고 업데이트합니다."));
  connect(flashPandaBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("판다를 플래시하시겠습니까?"), tr("플래시"), this)) {
      std::thread([=]() {
        QProcess process;
        flashPandaBtn->setValue(tr("플래시..."));

        process.setWorkingDirectory("/data/openpilot/panda/board");
        process.start("/bin/sh", QStringList{"-c", "./recover.py"});
        process.waitForFinished();
        process.start("/bin/sh", QStringList{"-c", "./flash.py"});
        process.waitForFinished();

        process.setWorkingDirectory("/data/openpilot/panda/tests");
        process.start("/bin/sh", QStringList{"-c", "python reflash_internal_panda.py"});
        process.waitForFinished();

        Hardware::reboot();
      }).detach();
    }
  });
  addItem(flashPandaBtn);

  QObject::connect(uiState(), &UIState::offroadTransition, [=](bool offroad) {
    for (auto btn : findChildren<ButtonControl *>()) {
      btn->setEnabled(offroad);
    }
  });

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton(tr("재부팅"));
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, this, &DevicePanel::reboot);

  QPushButton *poweroff_btn = new QPushButton(tr("전원 종료"));
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, this, &DevicePanel::poweroff);

  if (!Hardware::PC()) {
    connect(uiState(), &UIState::offroadTransition, poweroff_btn, &QPushButton::setVisible);
  }

  setStyleSheet(R"(
    #reboot_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { height: 120px; border-radius: 15px; background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  addItem(power_layout);
}

void DevicePanel::updateCalibDescription() {
  QString desc =
      tr("openpilot은 장치를 좌우 4° 이내, "
        "위로 5° 또는 아래로 9° 이내로 장착해야 합니다. openpilot은 지속적으로 교정되므로 재설정이 거의 필요하지 않습니다.");
  std::string calib_bytes = params.get("CalibrationParams");
  if (!calib_bytes.empty()) {
    try {
      AlignedBuffer aligned_buf;
      capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
      auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
      if (calib.getCalStatus() != cereal::LiveCalibrationData::Status::UNCALIBRATED) {
        double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
        double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
        desc += tr(" \n장치가 %1° %2 그리고 %3° %4 위치해 있습니다.")
                    .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "위로" : "아래로",
                         QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "오른쪽으로" : "왼쪽으로");
      }
    } catch (kj::Exception) {
      qInfo() << "invalid CalibrationParams";
    }
  }
  qobject_cast<ButtonControl *>(sender())->setDescription(desc);
}

void DevicePanel::reboot() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("재부팅을 하시겠습니까?"), tr("재부팅"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoReboot", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("Disengage to Reboot"), this);
  }
}

void DevicePanel::poweroff() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("전원을 종료 하시겠습니까?"), tr("전원 종료"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoShutdown", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("Disengage to Power Off"), this);
  }
}

void SettingsWindow::hideEvent(QHideEvent *event) {
  closeParentToggle();

  parentToggleOpen = false;
  subParentToggleOpen = false;
  subSubParentToggleOpen = false;

  previousScrollPosition = 0;
}

void SettingsWindow::showEvent(QShowEvent *event) {
  setCurrentPanel(0);
}

void SettingsWindow::setCurrentPanel(int index, const QString &param) {
  panel_widget->setCurrentIndex(index);
  nav_btns->buttons()[index]->setChecked(true);
  if (!param.isEmpty()) {
    emit expandToggleDescription(param);
  }
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  panel_widget = new QStackedWidget();

  // close button
  QPushButton *close_btn = new QPushButton(tr("← Back"));
  close_btn->setStyleSheet(R"(
    QPushButton {
      color: white;
      border-radius: 25px;
      background: #292929;
      font-size: 50px;
      font-weight: 500;
    }
    QPushButton:pressed {
      color: #ADADAD;
    }
  )");
  close_btn->setFixedSize(300, 125);
  sidebar_layout->addSpacing(10);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignRight);
  QObject::connect(close_btn, &QPushButton::clicked, [this]() {
    if (subSubParentToggleOpen) {
      closeSubSubParentToggle();
      subSubParentToggleOpen = false;
    } else if (subParentToggleOpen) {
      closeSubParentToggle();
      subParentToggleOpen = false;
    } else if (parentToggleOpen) {
      closeParentToggle();
      parentToggleOpen = false;
    } else {
      closeSettings();
    }
  });

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  TogglesPanel *toggles = new TogglesPanel(this);
  QObject::connect(this, &SettingsWindow::expandToggleDescription, toggles, &TogglesPanel::expandToggleDescription);
  QObject::connect(toggles, &TogglesPanel::updateMetric, this, &SettingsWindow::updateMetric);

  FrogPilotControlsPanel *frogpilotControls = new FrogPilotControlsPanel(this);
  QObject::connect(frogpilotControls, &FrogPilotControlsPanel::openParentToggle, this, [this]() {parentToggleOpen=true;});
  QObject::connect(frogpilotControls, &FrogPilotControlsPanel::openSubParentToggle, this, [this]() {subParentToggleOpen=true;});
  QObject::connect(frogpilotControls, &FrogPilotControlsPanel::openSubSubParentToggle, this, [this]() {subSubParentToggleOpen=true;});

  FrogPilotVisualsPanel *frogpilotVisuals = new FrogPilotVisualsPanel(this);
  QObject::connect(frogpilotVisuals, &FrogPilotVisualsPanel::openParentToggle, this, [this]() {parentToggleOpen=true;});

  QList<QPair<QString, QWidget *>> panels = {
    {tr("Device"), device},
    {tr("Network"), new Networking(this)},
    {tr("Toggles"), toggles},
    {tr("Software"), new SoftwarePanel(this)},
    {tr("컨트롤"), frogpilotControls},
    {tr("네비게이션"), new FrogPilotNavigationPanel(this)},
    {tr("차량"), new FrogPilotVehiclesPanel(this)},
    {tr("화면"), frogpilotVisuals},
  };

  nav_btns = new QButtonGroup(this);
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 65px;
        font-weight: 500;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )");
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    const int lr_margin = name != tr("Network") ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    if (name == tr("Controls") || name == tr("Visuals")) {
      QScrollBar *scrollbar = panel_frame->verticalScrollBar();

      QObject::connect(scrollbar, &QScrollBar::valueChanged, this, [this](int value) {
        if (!parentToggleOpen) {
          previousScrollPosition = value;
        }
      });

      QObject::connect(scrollbar, &QScrollBar::rangeChanged, this, [this, panel_frame]() {
        if (!parentToggleOpen) {
          panel_frame->restorePosition(previousScrollPosition);
        }
      });
    }

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      closeParentToggle();
      previousScrollPosition = 0;
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
    QStackedWidget, ScrollView {
      background-color: #292929;
      border-radius: 30px;
    }
  )");
}
