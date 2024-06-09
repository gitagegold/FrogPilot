#include "selfdrive/frogpilot/ui/qt/offroad/visual_settings.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  std::string branch = params.get("GitBranch");
  isRelease = branch == "FrogPilot";

  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"AlertVolumeControl", tr("경보 볼륨 컨트롤러"), tr("Openpilot에서 각 개별 사운드의 볼륨 레벨을 제어합니다."), "../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"DisengageVolume", "비작동 볼륨 ", "관련 경고:\n\n어댑티브 크루즈 비활성화\n주차 브레이크 작동\n페달 밟음\n속도가 너무 낮음", ""},
    {"EngageVolume", "작동 볼륨", "관련 경고:\n\nNNFF 토크 컨트롤러 로드됨", ""},
    {"PromptVolume", "프롬프트 볼륨", "관련 경고:\n\n사각지대에서 차량이 감지됨\n신호등이 녹색으로 바뀌었습니다\n속도가 너무 낮습니다\n'X' 이하에서는 조향을 사용할 수 없습니다\n조종권을 확보하고 조향 한계를 초과합니다.", ""},
    {"PromptDistractedVolume", "신속한 산만한 볼륨", "관련 경고:\n\n주의, 운전자 주의 분산\n핸들 터치, 운전자 무반응", ""},
    {"RefuseVolume", "거부 볼륨", "관련 경고:\n\nopenpilot 사용할 수 없음", ""},
    {"WarningSoftVolume", "경고 소프트 볼륨", "관련 경고:\n\n브레이크!, 충돌 위험\n즉시 제어하십시오", ""},
    {"WarningImmediateVolume", "즉시 볼륨 경고", "관련 경고:\n\n즉시 해제, 운전자 주의가 산만해짐\n즉시 해제, 운전자 응답 없음", ""},

    {"CustomAlerts", tr("사용자 정의 알림"), tr("openpilot 이벤트에 대한 사용자 정의 알림을 활성화합니다."), "../frogpilot/assets/toggle_icons/icon_green_light.png"},
    {"GreenLightAlert", tr("출발 신호등 알림"), tr("신호등이 빨간불에서 초록불로 바뀌면 알림을 받으세요."), ""},
    {"LeadDepartingAlert", tr("선행차량 출발 알림"), tr("정지한 상태에서 선행 차량이 출발을 시작하면 알림을 받습니다."), ""},
    {"LoudBlindspotAlert", tr("큰 소리로 사각지대 경고"), tr("차선을 변경하려고 할 때 사각지대에 차량이 감지되면 더 큰 소리로 경고합니다."), ""},

    {"CustomUI", tr("사용자 정의 주행 UI"), tr("사용자 정의 주행 UI."), "../assets/offroad/icon_road.png"},
    {"Compass", "나침반", "온로드 UI에 나침반을 추가하세요.", "../frogpilot/assets/toggle_icons/icon_compass.png"},
    {"CustomPaths", tr("경로"), tr("주행 경로, 감지된 인접 차선 또는 사각지대에서 차량이 감지될 때 예상 가속도를 표시합니다."), ""},
    {"PedalsOnUI", tr("페달 눌림"), tr("스티어링 휠 아이콘 아래의 도로 UI에 브레이크와 가스 페달을 표시합니다."), ""},
    {"RoadNameUI", tr("도로 이름"), tr("화면 하단에 현재 도로 이름을 표시합니다. OpenStreetMap에서 가져왔습니다."), ""},
    {"WheelIcon", tr("스티어링 휠 아이콘"), tr("기본 스티어링 휠 아이콘을 사용자 지정 아이콘으로 바꿉니다."), ""},

    {"CustomTheme", "사용자 정의 테마", "사용자 정의 테마를 사용하는 기능을 활성화합니다.", "../frogpilot/assets/wheel_images/frog.png"},
    {"CustomColors", "색상 테마", "표준 Openpilot 색 구성표를 사용자 지정 색 구성표로 전환하세요.\n\n자신만의 색 구성표를 제출하고 싶으십니까? FrogPilot Discord의 '기능 요청' 채널에 게시하세요!", ""},
    {"CustomIcons", "아이콘 팩", "표준 Openpilot 아이콘을 사용자 정의 아이콘 세트로 전환하세요.\n\n자신만의 아이콘 팩을 제출하고 싶으십니까? FrogPilot Discord의 '기능 요청' 채널에 게시하세요!", ""},
    {"CustomSounds", "사운드 팩", "표준 오픈파일럿 사운드를 사용자 정의 사운드 세트로 전환하세요.\n\n자신만의 사운드 팩을 제출하고 싶으십니까? FrogPilot Discord의 '기능 요청' 채널에 게시하세요!", ""},
    {"CustomSignals", "방향 지시등", "개인적인 터치를 위해 방향 지시등에 사용자 정의 애니메이션을 추가하세요!\n\n자신만의 방향 지시등 애니메이션을 제출하고 싶으십니까? FrogPilot Discord의 '기능 요청' 채널에 게시하세요!", ""},
    {"HolidayThemes", tr("휴일 테마"), tr("openpilot 테마는 현재/다가오는 휴일에 따라 변경됩니다. 사소한 휴일은 하루 동안 지속되고 주요 휴일(부활절, 크리스마스, 할로윈 등)은 일주일 동안 지속됩니다."), ""},
    {"RandomEvents", tr("임의의 이벤트"), tr("특정 주행 조건에서 발생할 수 있는 무작위 이벤트로 약간의 예측 불가능성을 즐기세요. 이는 순전히 미용적일 뿐 주행 제어에는 영향을 미치지 않습니다!"), ""},

    {"DeveloperUI", tr("개발자 UI"), tr("openpilot이 백그라운드에서 수행하는 다양한 자세한 정보를 얻으세요."), "../frogpilot/assets/toggle_icons/icon_device.png"},
    {"BorderMetrics", tr("테두리 숫자"), tr("onroad UI 테두리에 메트릭 표시."), ""},
    {"FPSCounter", tr("FPS 카운터"), tr("시스템 성능을 모니터링하기 위해 온로드 UI의 '초당 프레임'(FPS)을 표시합니다."), ""},
    {"LateralMetrics", tr("측면 메트릭"), tr("openpilot의 측면 성능과 관련된 다양한 메트릭을 표시합니다."), ""},
    {"LongitudinalMetrics", tr("종방향 메트릭"), tr("openpilot의 종방향 성능과 관련된 다양한 메트릭을 표시합니다."), ""},
    {"NumericalTemp", tr("숫자 온도 게이지"), tr("메모리, CPU, GPU 중 가장 높은 온도를 기준으로 하는 숫자 온도 게이지로 'GOOD', 'OK', 'HIGH' 온도 상태를 대체합니다."), ""},
    {"SidebarMetrics", tr("사이드바"), tr("다양한 사용자 정의 사이드바에 CPU, GPU, RAM, IP, 사용/남은 스토리지에 대한 메트릭을 표시합니다."), ""},
    {"UseSI", tr("국제 단위계 사용"), tr("SI 형식으로 관련 메트릭을 표시합니다."), ""},

    {"ModelUI", "모델 UI", "모델의 시각화가 화면에 표시되는 방식을 개인화하세요.", "../assets/offroad/icon_calibration.png"},
    {"DynamicPathWidth", "동적 경로 너비", "openpilot의 현재 참여 상태에 따라 경로 너비를 동적으로 조정합니다.", ""},
    {"HideLeadMarker", "선행차량 마커 숨기기", "온로드 UI에서 선행 차량 마커를 숨깁니다.", ""},
    {"LaneLinesWidth", "차선 선", "디스플레이에서 차선의 시각적 두께를 조정합니다.\n\n기본값은 MUTCD 평균 4인치와 일치합니다.", ""},
    {"PathEdgeWidth", "경로 가장자리", "UI에 표시되는 경로 가장자리의 너비를 조정하여 다양한 운전 모드와 상태를 나타냅니다.\n\n기본값은 전체 경로의 20%입니다.\n\n파란색 = 내비게이션\n밝은 파란색 = 항상 켜짐 측면\n녹색 = 기본값 'FrogPilot 색상'\n밝은 녹색 = 기본 색상의 기본값\n주황색 = 실험 모드 활성화\n노란색 = 조건부 재정의", ""},
    {"PathWidth", "경로 너비", "UI에 표시되는 주행 경로의 너비를 맞춤설정하세요.\n\n기본값은 2019 Lexus ES 350의 너비와 일치합니다.", ""},
    {"RoadEdgesWidth", "도로 가장자리", "디스플레이에서 도로 가장자리의 시각적 두께를 조정합니다.\n\n기본값은 MUTCD 평균 차선 폭 4인치의 1/2입니다.", ""},
    {"UnlimitedLength", "'무제한' 도로 UI 길이", "시스템이 감지할 수 있는 만큼 경로, 차선, 도로 가장자리 표시를 확장하여 전방 도로에 대한 더 넓은 시야를 제공합니다.", ""},

    {"QOLVisuals", tr("삶의 질"), tr("전반적인 openpilot 경험을 개선하기 위한 다양한 삶의 질 변화."), "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"BigMap", tr("큰 지도"), tr("도로 UI에서 지도 크기를 늘립니다."), ""},
    {"CameraView", tr("카메라 뷰"), tr("도로 UI에 대한 선호하는 카메라 뷰를 선택합니다. 이것은 순전히 시각적인 변화이며 openpilot 주행 방식에 영향을 미치지 않습니다."), ""},
    {"DriverCamera", tr("후진 시 운전자 카메라"), tr("후진 시 운전자 카메라 피드를 표시합니다."), ""},
    {"HideSpeed", tr("속도 숨기기"), tr("도로 UI에서 속도 표시기를 숨깁니다. 추가 토글을 사용하면 속도 자체를 탭하여 숨기거나 표시합니다."), ""},
    {"MapStyle", tr("지도 스타일"), tr("내비게이션에 사용할 지도 스타일을 선택합니다."), ""},
    {"WheelSpeed", tr("휠 속도 사"), tr("온로드 UI에서 클러스터 속도 대신 휠 속도를 사용합니다."), ""},

    {"ScreenManagement", "화면 관리", "화면 밝기, 시간 제한 설정을 관리하고 특정 온로드 UI 요소를 숨깁니다.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"HideUIElements", "UI 요소 숨기기", "온로드 화면에서 선택한 UI 요소를 숨깁니다..", ""},
    {"ScreenBrightness", "화면 밝기", "오프로드 시 화면 밝기를 사용자 정의하세요.", ""},
    {"ScreenBrightnessOnroad", "화면 밝기(온로드)", "온로드 시 화면 밝기를 사용자 정의하세요.", ""},
    {"ScreenRecorder", "화면 녹화", "화면을 녹화하려면 화면 녹화 버튼을 활성화하세요.", ""},
    {"ScreenTimeout", "화면 꺼짐", "화면이 꺼지는 데 걸리는 시간을 사용자 정의하세요.", ""},
    {"ScreenTimeoutOnroad", "온로드시 화면 꺼짐", "도로 주행 후 화면이 꺼지는 데 걸리는 시간을 맞춤설정하세요.", ""},
    {"StandbyMode", "대기 모드", "도로에서 화면 시간이 초과되면 화면을 끄고 참여 상태가 변경되거나 중요한 알림이 실행되면 다시 깨우세요.", ""},
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    AbstractControl *toggle;

    if (param == "AlertVolumeControl") {
      FrogPilotParamManageControl *alertVolumeControlToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(alertVolumeControlToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(alertVolumeControlKeys.find(key.c_str()) != alertVolumeControlKeys.end());
        }
      });
      toggle = alertVolumeControlToggle;
    } else if (alertVolumeControlKeys.find(param) != alertVolumeControlKeys.end()) {
      if (param == "WarningImmediateVolume") {
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 25, 100, std::map<int, QString>(), this, false, "%");
      } else {
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, "%");
      }

    } else if (param == "CustomAlerts") {
      FrogPilotParamManageControl *customAlertsToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customAlertsToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedCustomAlertsKeys = customAlertsKeys;

          if (!hasBSM) {
            modifiedCustomAlertsKeys.erase("LoudBlindspotAlert");
          }

          toggle->setVisible(modifiedCustomAlertsKeys.find(key.c_str()) != modifiedCustomAlertsKeys.end());
        }
      });
      toggle = customAlertsToggle;

    } else if (param == "CustomTheme") {
      FrogPilotParamManageControl *customThemeToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customThemeToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customThemeKeys.find(key.c_str()) != customThemeKeys.end());
        }
      });
      toggle = customThemeToggle;
    } else if (param == "CustomColors" || param == "CustomIcons" || param == "CustomSignals" || param == "CustomSounds") {
      std::vector<QString> themeOptions{tr("Stock"), tr("Frog"), tr("Tesla"), tr("Stalin")};
      FrogPilotButtonParamControl *themeSelection = new FrogPilotButtonParamControl(param, title, desc, icon, themeOptions);
      toggle = themeSelection;

      if (param == "CustomSounds") {
        QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
          if (id == 1) {
            if (FrogPilotConfirmationDialog::yesorno(tr("Do you want to enable the bonus 'Goat' sound effect?"), this)) {
              params.putBoolNonBlocking("GoatScream", true);
            } else {
              params.putBoolNonBlocking("GoatScream", false);
            }
          }
        });
      }

    } else if (param == "CustomUI") {
      FrogPilotParamManageControl *customUIToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customUIToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedCustomOnroadUIKeys = customOnroadUIKeys;

          if (!hasOpenpilotLongitudinal && !hasAutoTune) {
            modifiedCustomOnroadUIKeys.erase("DeveloperUI");
          }

          toggle->setVisible(modifiedCustomOnroadUIKeys.find(key.c_str()) != modifiedCustomOnroadUIKeys.end());
        }
      });
      toggle = customUIToggle;
    } else if (param == "CustomPaths") {
      std::vector<QString> pathToggles{"AccelerationPath", "AdjacentPath", "BlindSpotPath", "AdjacentPathMetrics"};
      std::vector<QString> pathToggleNames{tr("Acceleration"), tr("Adjacent"), tr("Blind Spot"), tr("Metrics")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, pathToggles, pathToggleNames);
    } else if (param == "PedalsOnUI") {
      std::vector<QString> pedalsToggles{"DynamicPedalsOnUI", "StaticPedalsOnUI"};
      std::vector<QString> pedalsToggleNames{tr("Dynamic"), tr("Static")};
      FrogPilotParamToggleControl *pedalsToggle = new FrogPilotParamToggleControl(param, title, desc, icon, pedalsToggles, pedalsToggleNames);
      QObject::connect(pedalsToggle, &FrogPilotParamToggleControl::buttonTypeClicked, this, [this, pedalsToggle](int index) {
        if (index == 0) {
          params.putBool("StaticPedalsOnUI", false);
        } else if (index == 1) {
          params.putBool("DynamicPedalsOnUI", false);
        }

        pedalsToggle->updateButtonStates();
      });
      toggle = pedalsToggle;

    } else if (param == "WheelIcon") {
      std::vector<QString> wheelToggles{"RotatingWheel"};
      std::vector<QString> wheelToggleNames{"Live Rotation"};
      std::map<int, QString> steeringWheelLabels = {{-1, tr("None")}, {0, tr("Stock")}, {1, tr("Lexus")}, {2, tr("Toyota")}, {3, tr("Frog")}, {4, tr("Rocket")}, {5, tr("Hyundai")}, {6, tr("Stalin")}};
      toggle = new FrogPilotParamValueToggleControl(param, title, desc, icon, -1, 6, steeringWheelLabels, this, true, "", 1, 1, wheelToggles, wheelToggleNames);

    } else if (param == "DeveloperUI") {
      FrogPilotParamManageControl *developerUIToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(developerUIToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedDeveloperUIKeys  = developerUIKeys ;

          toggle->setVisible(modifiedDeveloperUIKeys.find(key.c_str()) != modifiedDeveloperUIKeys.end());
        }
      });
      toggle = developerUIToggle;
    } else if (param == "BorderMetrics") {
      std::vector<QString> borderToggles{"BlindSpotMetrics", "ShowSteering", "SignalMetrics"};
      std::vector<QString> borderToggleNames{tr("Blind Spot"), tr("Steering Torque"), tr("Turn Signal"), };
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, borderToggles, borderToggleNames);
    } else if (param == "NumericalTemp") {
      std::vector<QString> temperatureToggles{"Fahrenheit"};
      std::vector<QString> temperatureToggleNames{tr("Fahrenheit")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, temperatureToggles, temperatureToggleNames);
    } else if (param == "SidebarMetrics") {
      std::vector<QString> sidebarMetricsToggles{"ShowCPU", "ShowGPU", "ShowIP", "ShowMemoryUsage", "ShowStorageLeft", "ShowStorageUsed"};
      std::vector<QString> sidebarMetricsToggleNames{tr("CPU"), tr("GPU"), tr("IP"), tr("RAM"), tr("SSD Left"), tr("SSD Used")};
      FrogPilotParamToggleControl *sidebarMetricsToggle = new FrogPilotParamToggleControl(param, title, desc, icon, sidebarMetricsToggles, sidebarMetricsToggleNames, this, 125);
      QObject::connect(sidebarMetricsToggle, &FrogPilotParamToggleControl::buttonTypeClicked, this, [this, sidebarMetricsToggle](int index) {
        if (index == 0) {
          params.putBool("ShowGPU", false);
        } else if (index == 1) {
          params.putBool("ShowCPU", false);
        } else if (index == 3) {
          params.putBool("ShowStorageLeft", false);
          params.putBool("ShowStorageUsed", false);
        } else if (index == 4) {
          params.putBool("ShowMemoryUsage", false);
          params.putBool("ShowStorageUsed", false);
        } else if (index == 5) {
          params.putBool("ShowMemoryUsage", false);
          params.putBool("ShowStorageLeft", false);
        }

        sidebarMetricsToggle->updateButtonStates();
      });
      toggle = sidebarMetricsToggle;

    } else if (param == "ModelUI") {
      FrogPilotParamManageControl *modelUIToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(modelUIToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedModelUIKeysKeys = modelUIKeys;

          if (!hasOpenpilotLongitudinal) {
            modifiedModelUIKeysKeys.erase("HideLeadMarker");
          }

          toggle->setVisible(modifiedModelUIKeysKeys.find(key.c_str()) != modifiedModelUIKeysKeys.end());
        }
      });
      toggle = modelUIToggle;
    } else if (param == "LaneLinesWidth" || param == "RoadEdgesWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 24, std::map<int, QString>(), this, false, tr(" inches"));
    } else if (param == "PathEdgeWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, tr("%"));
    } else if (param == "PathWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, tr(" feet"), 10);

    } else if (param == "QOLVisuals") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(qolKeys.find(key.c_str()) != qolKeys.end());
        }
      });
      toggle = qolToggle;
    } else if (param == "CameraView") {
      std::vector<QString> cameraOptions{tr("Auto"), tr("Driver"), tr("Standard"), tr("Wide")};
      FrogPilotButtonParamControl *preferredCamera = new FrogPilotButtonParamControl(param, title, desc, icon, cameraOptions);
      toggle = preferredCamera;
    } else if (param == "BigMap") {
      std::vector<QString> mapToggles{"FullMap"};
      std::vector<QString> mapToggleNames{tr("Full Map")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, mapToggles, mapToggleNames);
    } else if (param == "HideSpeed") {
      std::vector<QString> hideSpeedToggles{"HideSpeedUI"};
      std::vector<QString> hideSpeedToggleNames{tr("Control Via UI")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, hideSpeedToggles, hideSpeedToggleNames);
    } else if (param == "MapStyle") {
      QMap<int, QString> styleMap = {
        {0, tr("Stock openpilot")},
        {1, tr("Mapbox Streets")},
        {2, tr("Mapbox Outdoors")},
        {3, tr("Mapbox Light")},
        {4, tr("Mapbox Dark")},
        {5, tr("Mapbox Satellite")},
        {6, tr("Mapbox Satellite Streets")},
        {7, tr("Mapbox Navigation Day")},
        {8, tr("Mapbox Navigation Night")},
        {9, tr("Mapbox Traffic Night")},
        {10, tr("mike854's (Satellite hybrid)")},
      };

      QStringList styles = styleMap.values();
      ButtonControl *mapStyleButton = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(mapStyleButton, &ButtonControl::clicked, [=]() {
        QStringList styles = styleMap.values();
        QString selection = MultiOptionDialog::getSelection(tr("Select a map style"), styles, "", this);
        if (!selection.isEmpty()) {
          int selectedStyle = styleMap.key(selection);
          params.putIntNonBlocking("MapStyle", selectedStyle);
          mapStyleButton->setValue(selection);
          updateFrogPilotToggles();
        }
      });

      int currentStyle = params.getInt("MapStyle");
      mapStyleButton->setValue(styleMap[currentStyle]);

      toggle = mapStyleButton;

    } else if (param == "ScreenManagement") {
      FrogPilotParamManageControl *screenToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(screenToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(screenKeys.find(key.c_str()) != screenKeys.end());
        }
      });
      toggle = screenToggle;
    } else if (param == "HideUIElements") {
      std::vector<QString> uiElementsToggles{"HideAlerts", "HideMapIcon", "HideMaxSpeed"};
      std::vector<QString> uiElementsToggleNames{tr("Alerts"), tr("Map Icon"), tr("Max Speed")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, uiElementsToggles, uiElementsToggleNames);
    } else if (param == "ScreenBrightness" || param == "ScreenBrightnessOnroad") {
      std::map<int, QString> brightnessLabels;
      if (param == "ScreenBrightnessOnroad") {
        for (int i = 0; i <= 101; i++) {
          brightnessLabels[i] = (i == 0) ? tr("Screen Off") : (i == 101) ? tr("Auto") : QString::number(i) + "%";
        }
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 101, brightnessLabels, this, false);
      } else {
        for (int i = 1; i <= 101; i++) {
          brightnessLabels[i] = (i == 101) ? tr("Auto") : QString::number(i) + "%";
        }
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 101, brightnessLabels, this, false);
      }
    } else if (param == "ScreenTimeout" || param == "ScreenTimeoutOnroad") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 5, 60, std::map<int, QString>(), this, false, tr(" seconds"));

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(static_cast<ToggleControl*>(toggle), &ToggleControl::toggleFlipped, &updateFrogPilotToggles);
    QObject::connect(static_cast<FrogPilotParamValueControl*>(toggle), &FrogPilotParamValueControl::valueChanged, &updateFrogPilotToggles);

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });

    QObject::connect(static_cast<FrogPilotParamManageControl*>(toggle), &FrogPilotParamManageControl::manageButtonClicked, [this]() {
      update();
    });
  }

  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotVisualsPanel::hideToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotVisualsPanel::updateMetric);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &FrogPilotVisualsPanel::updateCarToggles);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotVisualsPanel::updateState);

  updateMetric();
}

void FrogPilotVisualsPanel::showEvent(QShowEvent *event) {
  hasOpenpilotLongitudinal = hasOpenpilotLongitudinal && !params.getBool("DisableOpenpilotLongitudinal");
}

void FrogPilotVisualsPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  started = s.scene.started;
}

void FrogPilotVisualsPanel::updateCarToggles() {
  auto carParams = params.get("CarParamsPersistent");
  if (!carParams.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(carParams.data(), carParams.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();
    auto carName = CP.getCarName();

    hasAutoTune = (carName == "hyundai" || carName == "toyota") && CP.getLateralTuning().which() == cereal::CarParams::LateralTuning::TORQUE;
    hasBSM = CP.getEnableBsm();
    hasOpenpilotLongitudinal = CP.getOpenpilotLongitudinalControl() && !params.getBool("DisableOpenpilotLongitudinal");
  } else {
    hasBSM = true;
    hasOpenpilotLongitudinal = true;
  }

  hideToggles();
}

void FrogPilotVisualsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? INCH_TO_CM : CM_TO_INCH;
    double speedConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    params.putIntNonBlocking("LaneLinesWidth", std::nearbyint(params.getInt("LaneLinesWidth") * distanceConversion));
    params.putIntNonBlocking("RoadEdgesWidth", std::nearbyint(params.getInt("RoadEdgesWidth") * distanceConversion));
    params.putIntNonBlocking("PathWidth", std::nearbyint(params.getInt("PathWidth") * speedConversion));
  }

  FrogPilotParamValueControl *laneLinesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["LaneLinesWidth"]);
  FrogPilotParamValueControl *roadEdgesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["RoadEdgesWidth"]);
  FrogPilotParamValueControl *pathWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["PathWidth"]);

  if (isMetric) {
    laneLinesWidthToggle->setDescription(tr("Customize the lane line width.\n\nDefault matches the Vienna average of 10 centimeters."));
    roadEdgesWidthToggle->setDescription(tr("Customize the road edges width.\n\nDefault is 1/2 of the Vienna average lane line width of 10 centimeters."));

    laneLinesWidthToggle->updateControl(0, 60, tr(" centimeters"));
    roadEdgesWidthToggle->updateControl(0, 60, tr(" centimeters"));
    pathWidthToggle->updateControl(0, 30, tr(" meters"), 10);
  } else {
    laneLinesWidthToggle->setDescription(tr("Customize the lane line width.\n\nDefault matches the MUTCD average of 4 inches."));
    roadEdgesWidthToggle->setDescription(tr("Customize the road edges width.\n\nDefault is 1/2 of the MUTCD average lane line width of 4 inches."));

    laneLinesWidthToggle->updateControl(0, 24, tr(" inches"));
    roadEdgesWidthToggle->updateControl(0, 24, tr(" inches"));
    pathWidthToggle->updateControl(0, 100, tr(" feet"), 10);
  }

  laneLinesWidthToggle->refresh();
  roadEdgesWidthToggle->refresh();
}

void FrogPilotVisualsPanel::hideToggles() {
  for (auto &[key, toggle] : toggles) {
    bool subToggles = alertVolumeControlKeys.find(key.c_str()) != alertVolumeControlKeys.end() ||
                      customAlertsKeys.find(key.c_str()) != customAlertsKeys.end() ||
                      customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end() ||
                      customThemeKeys.find(key.c_str()) != customThemeKeys.end() ||
                      developerUIKeys.find(key.c_str()) != developerUIKeys.end() ||
                      modelUIKeys.find(key.c_str()) != modelUIKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end() ||
                      screenKeys.find(key.c_str()) != screenKeys.end();
    toggle->setVisible(!subToggles);
  }

  update();
}
