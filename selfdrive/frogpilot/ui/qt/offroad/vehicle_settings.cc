#include <QDir>
#include <QRegularExpression>
#include <QTextStream>

#include "selfdrive/frogpilot/ui/qt/offroad/vehicle_settings.h"

QStringList getCarNames(const QString &carMake) {
  QMap<QString, QString> makeMap;
  makeMap["acura"] = "honda";
  makeMap["audi"] = "volkswagen";
  makeMap["buick"] = "gm";
  makeMap["cadillac"] = "gm";
  makeMap["chevrolet"] = "gm";
  makeMap["chrysler"] = "chrysler";
  makeMap["dodge"] = "chrysler";
  makeMap["ford"] = "ford";
  makeMap["gm"] = "gm";
  makeMap["gmc"] = "gm";
  makeMap["genesis"] = "hyundai";
  makeMap["honda"] = "honda";
  makeMap["hyundai"] = "hyundai";
  makeMap["infiniti"] = "nissan";
  makeMap["jeep"] = "chrysler";
  makeMap["kia"] = "hyundai";
  makeMap["lexus"] = "toyota";
  makeMap["lincoln"] = "ford";
  makeMap["man"] = "volkswagen";
  makeMap["mazda"] = "mazda";
  makeMap["nissan"] = "nissan";
  makeMap["ram"] = "chrysler";
  makeMap["seat"] = "volkswagen";
  makeMap["škoda"] = "volkswagen";
  makeMap["subaru"] = "subaru";
  makeMap["tesla"] = "tesla";
  makeMap["toyota"] = "toyota";
  makeMap["volkswagen"] = "volkswagen";

  QString dirPath = "../car";
  QDir dir(dirPath);
  QString targetFolder = makeMap.value(carMake, carMake);
  QStringList names;

  QString filePath = dir.absoluteFilePath(targetFolder + "/values.py");
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    QRegularExpression regex(R"delimiter(\w+\s*=\s*\w+PlatformConfig\(\s*"([^"]+)",)delimiter");
    QRegularExpressionMatchIterator it = regex.globalMatch(in.readAll());
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      names << match.captured(1);
    }
    file.close();
  }

  std::sort(names.begin(), names.end());
  return names;
}

FrogPilotVehiclesPanel::FrogPilotVehiclesPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  selectMakeButton = new ButtonControl(tr("자동차 회사 선택"), tr("선택"));
  QObject::connect(selectMakeButton, &ButtonControl::clicked, [this]() {
    QStringList makes = {
      "Acura", "Audi", "BMW", "Buick", "Cadillac", "Chevrolet", "Chrysler", "Dodge", "Ford", "GM", "GMC",
      "Genesis", "Honda", "Hyundai", "Infiniti", "Jeep", "Kia", "Lexus", "Lincoln", "MAN", "Mazda",
      "Mercedes", "Nissan", "Ram", "SEAT", "Škoda", "Subaru", "Tesla", "Toyota", "Volkswagen", "Volvo",
    };

    QString newMakeSelection = MultiOptionDialog::getSelection(tr("자동차 회사 선택"), makes, "", this);
    if (!newMakeSelection.isEmpty()) {
      carMake = newMakeSelection;
      params.putNonBlocking("CarMake", carMake.toStdString());
      selectMakeButton->setValue(newMakeSelection);
      setModels();
    }
  });
  addItem(selectMakeButton);

  selectModelButton = new ButtonControl(tr("모델 선택"), tr("선택"));
  QObject::connect(selectModelButton, &ButtonControl::clicked, [this]() {
    QString newModelSelection = MultiOptionDialog::getSelection(tr("모델 선택"), models, "", this);
    if (!newModelSelection.isEmpty()) {
      carModel = newModelSelection;
      params.putNonBlocking("CarModel", newModelSelection.toStdString());
      selectModelButton->setValue(newModelSelection);
    }
  });
  addItem(selectModelButton);
  selectModelButton->setVisible(false);

  ParamControl *forceFingerprint = new ParamControl("ForceFingerprint", tr("자동 지문 감지 비활성화"), tr("선택한 지문을 강제로 변경하고 영구적으로 변경되지 않도록 방지합니다."), "", this);
  addItem(forceFingerprint);

  bool disableOpenpilotLongState = params.getBool("DisableOpenpilotLongitudinal");
  disableOpenpilotLong = new ToggleControl(tr("Openpilot 종방향 제어 비활성화"), tr("오픈 파일럿 종방향 제어를 비활성화하고 대신 스톡 ACC를 사용하세요."), "", disableOpenpilotLongState);
  QObject::connect(disableOpenpilotLong, &ToggleControl::toggleFlipped, [=](bool state) {
    if (state) {
      if (FrogPilotConfirmationDialog::yesorno(tr("Openpilot 종방향 제어를 완전히 비활성화하시겠습니까?"), this)) {
        params.putBoolNonBlocking("DisableOpenpilotLongitudinal", state);
        if (started) {
          if (FrogPilotConfirmationDialog::toggle(tr("적용하려면 재부팅이 필요합니다."), tr("Reboot Now"), this)) {
            Hardware::reboot();
          }
        }
      }
    } else {
      params.putBoolNonBlocking("DisableOpenpilotLongitudinal", state);
    }
    updateCarToggles();
  });
  addItem(disableOpenpilotLong);

  std::vector<std::tuple<QString, QString, QString, QString>> vehicleToggles {
    {"LongPitch", tr("롱 피치 보상"), tr("가스 및 페달 컨트롤을 부드럽게 만드세요."), ""},
    {"GasRegenCmd", tr("트럭 튜닝"), tr("정지할 때 가속도를 높이고 브레이크 제어를 부드럽게 합니다. Silverado/Sierra에서만 사용 가능."), ""},

    {"CrosstrekTorque", tr("Subaru Crosstrek Torque Increase"), tr("Increases the maximum allowed torque for the Subaru Crosstrek."), ""},

    {"ToyotaDoors", tr("자동으로 문 잠금/잠금 해제"), tr("주행 중일 때는 자동으로 문을 잠그고 주차 중일 때는 문을 잠금 해제합니다."), ""},
    {"ClusterOffset", tr("클러스터 오프셋"), tr("대시보드에 표시된 속도와 일치하도록 Openpilot에서 사용하는 클러스터 오프셋을 설정합니다."), ""},
    {"SNGHack", tr("정지 및 출발 해킹"), tr("정지 및 이동 기능이 없는 차량에 대해 '정지 및 이동' 해킹을 활성화합니다."), ""},
    {"ToyotaTune", tr("Toyota Tune"), tr("사용자 지정 Toyota 종방향 튜닝을 사용합니다.\n\nCydia = TSS-P 차량에 더 중점을 두었지만 모든 Toyota에서 작동합니다.\n\nDragonPilot = TSS2 차량에 중점을 둡니다.\n\nFrogPilot = 두 가지의 장점을 모두 취하고 개인적인 조정을 중심으로 합니다. FrogsGoMoo의 2019 Lexus ES 350"), ""},
  };

  for (const auto &[param, title, desc, icon] : vehicleToggles) {
    AbstractControl *toggle;

    if (param == "ToyotaDoors") {
      std::vector<QString> lockToggles{"LockDoors", "UnlockDoors"};
      std::vector<QString> lockToggleNames{tr("Lock"), tr("Unlock")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, lockToggles, lockToggleNames);

    } else if (param == "ClusterOffset") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1.000, 1.050, std::map<int, QString>(), this, false, "x", 1, 0.001);

    } else if (param == "ToyotaTune") {
      std::vector<std::pair<QString, QString>> tuneOptions{
        {"StockTune", tr("Stock")},
        {"CydiaTune", tr("Cydia")},
        {"DragonPilotTune", tr("DragonPilot")},
        {"FrogsGoMooTune", tr("FrogPilot")},
      };
      toggle = new FrogPilotButtonsParamControl(param, title, desc, icon, tuneOptions);

      QObject::connect(static_cast<FrogPilotButtonsParamControl*>(toggle), &FrogPilotButtonsParamControl::buttonClicked, [this]() {
        if (started) {
          if (FrogPilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
            Hardware::reboot();
          }
        }
      });

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    toggle->setVisible(false);
    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(static_cast<ToggleControl*>(toggle), &ToggleControl::toggleFlipped, &updateFrogPilotToggles);

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  std::set<QString> rebootKeys = {"CrosstrekTorque", "GasRegenCmd"};
  for (const QString &key : rebootKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key.toStdString().c_str()]), &ToggleControl::toggleFlipped, [this]() {
      if (started) {
        if (FrogPilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
          Hardware::reboot();
        }
      }
    });
  }

  QObject::connect(uiState(), &UIState::offroadTransition, [this](bool offroad) {
    std::thread([this]() {
      while (carMake.isEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        carMake = QString::fromStdString(params.get("CarMake"));
        carModel = QString::fromStdString(params.get("CarModel"));
      }
      setModels();
      updateCarToggles();
    }).detach();
  });

  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotVehiclesPanel::updateState);

  carMake = QString::fromStdString(params.get("CarMake"));
  carModel = QString::fromStdString(params.get("CarModel"));

  if (!carMake.isEmpty()) {
    setModels();
  }
}

void FrogPilotVehiclesPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  started = s.scene.started;
}

void FrogPilotVehiclesPanel::updateCarToggles() {
  auto carParams = params.get("CarParamsPersistent");
  if (!carParams.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(carParams.data(), carParams.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();

    auto carFingerprint = CP.getCarFingerprint();

    hasExperimentalOpenpilotLongitudinal = CP.getExperimentalLongitudinalAvailable();
    hasOpenpilotLongitudinal = CP.getOpenpilotLongitudinalControl();
    hasSNG = CP.getMinEnableSpeed() <= 0;
    isGMTruck = carFingerprint == "CHEVROLET SILVERADO 1500 2020";
    isImpreza = carFingerprint == "SUBARU IMPREZA LIMITED 2019";
  } else {
    hasExperimentalOpenpilotLongitudinal = false;
    hasOpenpilotLongitudinal = true;
    hasSNG = false;
    isGMTruck = true;
    isImpreza = true;
  }

  hideToggles();
}

void FrogPilotVehiclesPanel::setModels() {
  models = getCarNames(carMake.toLower());
  hideToggles();
}

void FrogPilotVehiclesPanel::hideToggles() {
  disableOpenpilotLong->setVisible(hasOpenpilotLongitudinal && !hasExperimentalOpenpilotLongitudinal && !params.getBool("HideDisableOpenpilotLongitudinal"));

  selectMakeButton->setValue(carMake);
  selectModelButton->setValue(carModel);
  selectModelButton->setVisible(!carMake.isEmpty());

  bool gm = carMake == "Buick" || carMake == "Cadillac" || carMake == "Chevrolet" || carMake == "GM" || carMake == "GMC";
  bool subaru = carMake == "Subaru";
  bool toyota = carMake == "Lexus" || carMake == "Toyota";

  std::set<QString> gmTruckKeys = {"GasRegenCmd"};
  std::set<QString> imprezaKeys = {"CrosstrekTorque"};
  std::set<QString> longitudinalKeys = {"GasRegenCmd", "ToyotaTune", "LongPitch", "SNGHack"};
  std::set<QString> sngKeys = {"SNGHack"};

  for (auto &[key, toggle] : toggles) {
    if (toggle) {
      toggle->setVisible(false);

      if ((!hasOpenpilotLongitudinal || params.getBool("DisableOpenpilotLongitudinal")) && longitudinalKeys.find(key.c_str()) != longitudinalKeys.end()) {
        continue;
      }

      if (hasSNG && sngKeys.find(key.c_str()) != sngKeys.end()) {
        continue;
      }

      if (!isGMTruck && gmTruckKeys.find(key.c_str()) != gmTruckKeys.end()) {
        continue;
      }

      if (!isImpreza && imprezaKeys.find(key.c_str()) != imprezaKeys.end()) {
        continue;
      }

      if (gm) {
        toggle->setVisible(gmKeys.find(key.c_str()) != gmKeys.end());
      } else if (subaru) {
        toggle->setVisible(subaruKeys.find(key.c_str()) != subaruKeys.end());
      } else if (toyota) {
        toggle->setVisible(toyotaKeys.find(key.c_str()) != toyotaKeys.end());
      }
    }
  }

  update();
}
