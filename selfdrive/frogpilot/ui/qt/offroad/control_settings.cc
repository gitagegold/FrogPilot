#include <filesystem>
#include <iostream>

#include "selfdrive/frogpilot/ui/qt/offroad/control_settings.h"

namespace fs = std::filesystem;

bool checkCommaNNFFSupport(const std::string &carFingerprint) {
  const std::string filePath = "../car/torque_data/neural_ff_weights.json";

  if (!std::filesystem::exists(filePath)) {
    return false;
  }

  std::ifstream file(filePath);
  std::string line;
  while (std::getline(file, line)) {
    if (line.find(carFingerprint) != std::string::npos) {
      std::cout << "comma's NNFF supports fingerprint: " << carFingerprint << std::endl;
      return true;
    }
  }

  return false;
}

bool checkNNFFLogFileExists(const std::string &carFingerprint) {
  const fs::path dirPath("../car/torque_data/lat_models");

  if (!fs::exists(dirPath)) {
    std::cerr << "Directory does not exist: " << fs::absolute(dirPath) << std::endl;
    return false;
  }

  for (const auto &entry : fs::directory_iterator(dirPath)) {
    if (entry.path().filename().string().find(carFingerprint) == 0) {
      std::cout << "NNFF supports fingerprint: " << entry.path().filename() << std::endl;
      return true;
    }
  }

  return false;
}

FrogPilotControlsPanel::FrogPilotControlsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  std::string branch = params.get("GitBranch");
  isRelease = branch == "FrogPilot";

  const std::vector<std::tuple<QString, QString, QString, QString>> controlToggles {
    {"AlwaysOnLateral", tr("상시 핸들 조향"), tr("브레이크 또는 가속 페달을 사용할때 오픈파일럿은 항상 핸들 조향을 합니다.\n\n핸들조향을 끌때는 크루즈 버튼을 사용하세요."), "../frogpilot/assets/toggle_icons/icon_always_on_lateral.png"},
    {"AlwaysOnLateralMain", tr("크루즈 컨트롤 사용"), tr("크루즈 컨트롤 키면 항상 핸들조향이 됩니다."), ""},
    {"PauseAOLOnBrake", tr("브레이크 아래 일시 정지"), tr("브레이크 페달이 설정 속도 이하로 눌려 있을 때 '항상 측면' 일시 정지"), ""},
    {"HideAOLStatusBar", tr("상태 표시줄 숨기기"), tr("상시 핸들 조향시 상태바를 사용 사용하지 마세요."), ""},

    {"ConditionalExperimental", tr("조건부 실험 모드"), tr("미리 정의된 조건에 따라 자동으로 '실험 모드'로 전환 됩니다."), "../frogpilot/assets/toggle_icons/icon_conditional.png"},
    {"CECurves", tr("전방에 커브 감지됨"), tr("커브가 감지되면 '실험 모드'로 전환됩니다."), ""},
    {"CENavigation", tr("네비게이션 기반"), tr("내비게이션 데이터(예: 교차로, 정지 신호 등)를 기반으로 '실험 모드'로 전환합니다.)"), ""},
    {"CESlowerLead", tr("전방에 느린 선행차 감지"), tr("앞서 느린 선두 차량이 감지되면 '실험 모드'로 전환합니다."), ""},
    {"CEStopLights", tr("정지 신호등 및 정지 신호"), tr("정지등이나 정지 신호가 감지되면 '실험 모드'로 전환합니다."), ""},
    {"CESignal", tr("고속도로 속도 이하일때 방향 지시등"), tr("회전을 보조하기 위해 고속도로 속도 이하에서 방향 지시등을 사용할 때 '실험 모드'로 전환하세요."), ""},
    {"HideCEMStatusBar", tr("상태 표시줄 숨기기"), tr("조건부 실험 모드에서 상태바를 사용하지 마세요."), ""},

    {"DeviceManagement", tr("장치 관리"), tr("개인 선호도에 맞게 장치의 동작을 조정합니다."), "../frogpilot/assets/toggle_icons/icon_device.png"},
    {"DeviceShutdown", tr("장치 종료 타이머"), tr("오프로드 후 장치가 얼마나 빨리 종료되는지 구성합니다."), ""},
    {"NoLogging", tr("로깅 비활성화"), tr("개인 정보 보호를 강화하거나 열 부하를 줄이려면 모든 데이터 추적을 끄십시오.\n\n경고: 이 조치는 드라이브 기록을 방지하며 데이터를 복구할 수 없습니다!"), ""},
    {"NoUploads", tr("업로드 비활성화"), tr("컴마의 서버로의 모든 데이터 업로드를 끕니다."), ""},
    {"IncreaseThermalLimits", tr("열 안전 한도 증가"), tr("장치가 컴마의 권장 열 한계입니다."), ""},
    {"LowVoltageShutdown", tr("저전압 종료 임계값"), tr("배터리가 특정 전압 수준에 도달하면 자동으로 기기를 종료하여 배터리가 죽는 것을 방지합니다."), ""},
    {"OfflineMode", tr("오프라인 모드"), tr("기기가 무기한 오프라인 상태가 되도록 허용합니다."), ""},

    {"DrivingPersonalities", tr("운전 성격"), tr("쉼표의 성격 프로필의 운전 행동을 관리합니다."), "../frogpilot/assets/toggle_icons/icon_personality.png"},
    {"CustomPersonalities", tr("운전스타일 사용자 설정"), tr("운전 성격 프로필을 운전 스타일에 맞게 사용자 지정합니다."), ""},
    {"TrafficPersonalityProfile", tr("교통 성격"), tr("교통 성격 프로필을 사용자 정의합니다."), "../frogpilot/assets/other_images/traffic.png"},
    {"TrafficFollow", tr("추종 거리"), tr("교통 모드를 사용할 때 최소 추종 거리를 설정합니다. 추종 거리는 0~%1 사이를 주행할 때 이 거리와 공격적 프로필의 추종 거리 사이에서 동적으로 조정됩니다.\n\n예:\n\n교통 모드: 0.5초\n공격적: 1.0초\n\n0%2 = 0.5초\n%3 = 0.75초\n%1 = 1.0초"), ""},
    {"TrafficJerkAcceleration", tr("가속/감속 응답 오프셋"), tr("교통 모드를 사용할 때 가속에 대한 응답 속도를 사용자 정의합니다"), ""},
    {"TrafficJerkSpeed", tr("속도 제어 응답 오프셋"), tr("트래픽 모드를 사용할 때 속도(브레이크 포함)를 유지하기 위한 응답 속도를 사용자 정의합니다."), ""},
    {"ResetTrafficPersonality", tr("설정 재설정"), tr("트래픽 모드 개성 값을 기본 설정으로 재설정합니다."), ""},
    {"AggressivePersonalityProfile", tr("공격적 성격"), tr("공격적 성격 프로필을 사용자 정의합니다."), "../frogpilot/assets/other_images/aggressive.png"},
    {"AggressiveFollow", tr("추종 거리"), tr("공격적 성격의 추종 거리를 설정합니다. 선두 차량 뒤를 따라가는 데 걸리는 시간을 초 단위로 나타냅니다.\n\n스톡: 1.25초"), ""},
    {"AggressiveJerkAcceleration", tr("가속/감속 반응 오프셋"), tr("공격적 성격을 사용할 때 가속에 대한 반응 속도를 사용자 정의합니다."), ""},
    {"AggressiveJerkSpeed", tr("속도 제어 반응 오프셋"), tr("공격적을 사용할 때 속도(브레이크 포함)를 유지하기 위한 반응 속도를 사용자 정의합니다"), ""},
    {"ResetAggressivePersonality", tr("설정 재설정"), tr("공격적 성격 값을 재고로 재설정합니다."), ""},
    {"StandardPersonalityProfile", tr("표준 성격"), tr("표준 성격 프로필을 사용자 지정합니다."), "../frogpilot/assets/other_images/standard.png"},
    {"StandardFollow", tr("추종 거리"), tr("표준 성격 추종 거리를 설정합니다. 선두 차량 뒤를 따라가는 초를 나타냅니다.\n\n스톡: 1.45초."), ""},
    {"StandardJerkAcceleration", tr("가속/감속 반응 오프셋"), tr("표준 성격을 사용할 때 가속에 대한 반응 속도를 사용자 정의합니다."), ""},
    {"StandardJerkSpeed", tr("속도 제어 반응 오프셋"), tr("Standard 성격을 사용할 때 속도(브레이크 포함)를 유지하기 위한 반응 속도를 사용자 정의합니다."), ""},
    {"ResetStandardPersonality", tr("설정 재설정"), tr("표준 성격의 값을 재고로 재설정합니다"), ""},
    {"RelaxedPersonalityProfile", tr("편안한 성격"), tr("편안한 성격 프로필을 사용자 정의합니다."), "../frogpilot/assets/other_images/relaxed.png"},
    {"RelaxedFollow", tr("추종 거리"), tr("편안한 성격의 추종 거리를 설정합니다. 선두 차량 뒤를 따라가는 데 걸리는 시간(초)을 나타냅니다.\n\nStock: 1.75초."), ""},
    {"RelaxedJerkAcceleration", tr("가속/감속 반응 오프셋"), tr("사용 시 가속에 대한 반응 속도를 사용자 정의합니다. 편안한 성격."), ""},
    {"RelaxedJerkSpeed", tr("속도 제어 반응 오프셋"), tr("편안한 성격을 사용할 때 속도(브레이크 포함)를 유지하기 위한 반응 속도를 사용자 정의합니다.."), ""},
    {"ResetRelaxedPersonality", tr("설정 재설정"), tr("편안한 성격의 값을 원래대로 재설정합니다.."), ""},
    {"OnroadDistanceButton", tr("Onroad 거리 버튼"), tr("온로드 UI를 통해 거리 버튼을 시뮬레이션하여 성격, '실험 모드' 및 '교통 모드'를 제어합니다."), ""},

    {"ExperimentalModeActivation", tr("실험 모드 활성화"), tr("스티어링 휠이나 화면의 버튼으로 실험 모드를 전환합니다. \n\n'조건부 실험 모드'를 재정의합니다."), "../assets/img_experimental_white.svg"},
    {"ExperimentalModeViaLKAS", tr("LKAS를 두 번 클릭합니다"), tr("스티어링 휠의 'LKAS' 버튼을 두 번 클릭하여 '실험 모드'를 활성화/비활성화합니다."), ""},
    {"ExperimentalModeViaTap", tr("UI를 두 번 탭합니다"), tr("0.5초 이내에 온로드 UI를 두 번 탭하여 '실험 모드'를 활성화/비활성화합니다."), ""},
    {"ExperimentalModeViaDistance", tr("길게 누를 수 있는 거리"), tr("활성화/비활성화 '실험 모드'로 전환하려면 스티어링 휠의 '거리' 버튼을 0.5초간 누릅니다."), ""},

    {"LaneChangeCustomizations", tr("차선 변경 사용자 지정"), tr("openpilot에서 차선 변경 동작을 사용자 지정합니다."), "../frogpilot/assets/toggle_icons/icon_lane.png"},
    {"MinimumLaneChangeSpeed", tr("최소 차선 변경 속도"), tr("openpilot에서 차선을 변경할 수 있도록 최소 주행 속도를 사용자 지정합니다."), ""},
    {"NudgelessLaneChange", tr("자동 차선 변경"), tr("수동 조향 입력 없이 차선 변경을 활성화합니다."), ""},
    {"LaneChangeTime", tr("차선 변경 타이머"), tr("차선 변경을 실행하기 전에 지연 시간을 설정합니다."), ""},
    {"LaneDetectionWidth", tr("차선 감지 임계값"), tr("차선으로 적격화되는 데 필요한 차선 너비를 설정합니다."), ""},
    {"OneLaneChange", tr("차선 변경 한 번당 신호"), tr("방향 지시등 작동 시 차선 변경은 한 번만 허용합니다.."), ""},

    {"LateralTune", tr("조향 튜닝"), tr("openpilot의 조향 동작을 수정합니다."), "../frogpilot/assets/toggle_icons/icon_lateral_tune.png"},
    {"ForceAutoTune", tr("강제 자동 튜닝"), tr("지원되지 않는 차량에 대해 쉼표의 자동 측면 튜닝을 강제합니다."), ""},
    {"NNFF", tr("NNFF"), tr("측면 제어의 정밀도를 높이기 위해 Twilsonco의 신경망 피드포워드를 사용합니다."), ""},
    {"NNFFLite", tr("NNFF-Lite"), tr("사용 가능한 NNFF 로그가 없는 차량에 대해 측면 제어의 정밀도를 높이기 위해 Twilsonco의 신경망 피드포워드를 사용합니다."), ""},
    {"SteerRatio", steerRatioStock != 0 ? QString(tr("스티어 비율 (Default: %1)")).arg(QString::number(steerRatioStock, 'f', 2)) : tr("Steer Ratio"), tr("컴마의 자동 튜닝 값 대신 사용자 지정 스티어 비율을 사용합니다."), ""},
    {"TacoTune", tr("Taco Tune"), tr("좌우 회전을 처리하도록 설계된 컴마의 'Taco Tune'을 사용합니다"), ""},
    {"TurnDesires", tr("턴 디자이어 사용"), tr("최소 차선 변경 속도 미만의 회전에서 더 정확한 회전을 위해 턴 디자이어를 사용합니다."), ""},

    {"LongitudinalTune", tr("종방향 튜닝"), tr("openpilot의 가속 및 제동 동작을 수정합니다."), "../frogpilot/assets/toggle_icons/icon_longitudinal_tune.png"},
    {"AccelerationProfile", tr("가속 프로필"), tr("가속 속도를 스포티하거나 친환경적으로 변경합니다."), ""},
    {"DecelerationProfile", tr("감속 프로필"), tr("감속 속도를 스포티하거나 친환경적으로 변경합니다."), ""},
    {"AggressiveAcceleration", tr("선두 차량 뒤에서 가속 증가"), tr("더 빠른 선두 차량을 따라갈 때 공격성을 높입니다."), ""},
    {"StoppingDistance", tr("선두 차량 뒤에서 정지 거리 증가"), tr("선두 차량에서 더 편안하게 정지할 수 있도록 정지 거리를 늘립니다."), ""},
    {"LeadDetectionThreshold", tr("선두 감지 임계값"), tr("선두 차량을 더 빨리 감지하거나 모델 신뢰도를 높이기 위해 리드 감지 임계값을 늘리거나 줄입니다."), ""},
    {"SmoothBraking", tr("더 부드러운 제동"), tr("느린 차량에 접근할 때 제동 동작을 부드럽게 합니다."), ""},
    {"TrafficMode", tr("교통 모드"), tr("거리 버튼을 2.5초 동안 눌러 '교통 모드'를 활성화하는 기능을 활성화합니다. '교통 모드'가 활성화되면 온로드 UI가 빨간색으로 바뀌고 오픈파일럿은 정지 및 이동 교통에 맞춰 운전합니다."), ""},

    {"MTSCEnabled", tr("맵 회전 속도 제어"), tr("다운로드한 맵에서 감지한 예상 곡선에 대해 속도를 줄입니다."), "../frogpilot/assets/toggle_icons/icon_speed_map.png"},
    {"DisableMTSCSmoothing", tr("맵 회전 속도 제어 UI 스무딩 비활성화"), tr("맵 회전 속도 제어가 현재 요청하는 속도를 정확히 표시하기 위해 도로 UI에서 요청한 속도에 대한 스무딩을 비활성화합니다."), ""},
    {"MTSCCurvatureCheck",  tr("모델 곡률 감지 실패 안전 장치"), tr("모델이 도로에서 곡선을 감지할 때만 MTSC를 트리거합니다. 거짓 양성을 방지하기 위한 실패 안전 장치로만 사용됩니다. 거짓 양성을 전혀 경험하지 않는 경우 이 기능을 끄세요."), ""},
    {"MTSCAggressiveness", tr("회전 속도 공격성"), tr("회전 속도 공격성 설정. 값이 높을수록 더 빠른 회전에서는 값이 낮을수록 회전이 더 부드럽습니다. \n\n+- 1% 변경하면 속도가 약 1mph 상승하거나 낮아집니다."), ""},

    {"ModelSelector", tr("모델 선택"), tr("openpilot의 주행 모델을 관리합니다."), "../assets/offroad/icon_calibration.png"},

    {"QOLControls", tr("삶의 질"), tr("전반적인 openpilot 경험을 개선하기 위한 다양한 삶의 질 변화."), "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"CustomCruise", tr("크루즈 증가 간격"), tr("최대 설정 속도를 증가시키는 사용자 지정 간격을 설정합니다."), ""},
    {"CustomCruiseLong", tr("크루즈 증가 간격(길게 누름)"), tr("최대 설정 속도를 증가시키는 사용자 지정 간격을 설정합니다.크루즈 증가 버튼을 누르고 있을 때 최대 설정 속도."), ""},
    {"MapGears", tr("가속/감속을 기어에 매핑"), tr("가속/감속 프로필을 'Eco' 및/또는 'Sport' 기어에 매핑합니다"), ""},
    {"PauseLateralSpeed", tr("핸들조향 아래 일시 중지"), tr("설정 속도 미만의 모든 속도에서 핸들 제어를 일시 중지합니다."), ""},
    {"ReverseCruise", tr("역 크루즈 증가"), tr("길게 누르기 기능 로직을 역으로 변경하여 최대 설정 속도를 1이 아닌 5만큼 증가시킵니다. 최대 속도를 빠르게 증가시키는 데 유용합니다."), ""},
    {"SetSpeedOffset", tr("속도 오프셋 설정"), tr("원하는 설정 속도에 대한 오프셋을 설정합니다."), ""},

    {"SpeedLimitController", tr("속도 제한 컨트롤러"), tr("현재 속도 제한과 일치하도록 최대 속도를 자동으로 조정합니다. 'Open Street Maps', 'Navigate On openpilot' 또는 자동차 대시보드(Toyotas/Lexus/HKG만 해당)를 사용합니다."), "../assets/offroad/icon_speed_limit.png"},
    {"SLCControls", tr("컨트롤 설정"), tr("속도 제한 컨트롤러의 컨트롤과 관련된 토글을 관리합니다."), ""},
    {"Offset1", tr("속도 제한 오프셋(0-34mph)"), tr("0-34mph 사이의 속도 제한에 대한 속도 제한 오프셋."), ""},
    {"Offset2", tr("속도 제한 오프셋(35-54mph)"), tr("35-54mph 사이의 속도 제한에 대한 속도 제한 오프셋."), ""},
    {"Offset3", tr("속도 제한 오프셋(55-64mph)"), tr("속도 제한 55-64mph에 대한 속도 제한 오프셋."), ""},
    {"Offset4", tr("속도 제한 오프셋(65-99mph)"), tr("속도 제한 65-99mph에 대한 속도 제한 오프셋."), ""},
    {"SLCFallback", tr("폴백 방법"), tr("사용 가능한 속도 제한이 없는 경우 폴백 방법을 선택하세요."), ""},
    {"SLCOverride", tr("오버라이드 방법"), tr("현재 속도 제한을 오버라이드할 선호하는 방법을 선택하세요."), ""},
    {"SLCPriority", tr("우선순위 순서"), tr("속도 제한 우선 순위 순서를 구성하세요."), ""},
    {"SLCQOL", tr("삶의 질 설정"), tr("속도 제한 컨트롤러의 삶의 질 기능과 관련된 토글을 관리합니다."), ""},
    {"SLCConfirmation", tr("새로운 속도 제한 확인"), tr("수동으로 확인되기 전까지는 새로운 속도 제한을 자동으로 사용하지 마십시오."), ""},
    {"ForceMPHDashboard", tr("대시보드 판독값에서 MPH 강제 적용"), tr("대시보드에서 MPH 판독값 강제 적용. 대시보드의 속도 제한이 KPH이지만 MPH를 사용하는 지역에 사는 경우에만 이 기능을 사용하세요."), ""},
    {"SLCLookaheadHigher", tr("더 높은 속도 제한에 대비"), tr("Open Street Maps에 저장된 데이터를 사용하여 현재 속도 제한보다 높은 속도 제한에 대비하기 위해 'lookahead' 값을 설정합니다."), ""},
    {"SLCLookaheadLower", tr("더 낮은 속도 제한에 대비"), tr("Open Street Maps에 저장된 데이터를 사용하여 현재 속도 제한보다 낮은 속도 제한에 대비하기 위해 'lookahead' 값을 설정합니다."), ""},
    {"SetSpeedLimit", tr("현재 속도 제한을 설정된 속도로 사용"), tr("최초로 openpilot을 활성화할 때 현재 속도 제한이 채워져 있으면 최대 속도를 현재 속도 제한으로 설정합니다."), ""},
    {"SLCVisuals", tr("Visuals 설정"), tr("Speed ​​Limit Controller's visuals'와 관련된 토글을 관리합니다."), ""},
    {"ShowSLCOffset", tr("속도 제한 오프셋 표시"), tr("속도 제한 컨트롤러를 사용할 때 도로 UI에서 속도 제한 오프셋을 속도 제한과 분리하여 표시합니다."), ""},
    {"SpeedLimitChangedAlert", tr("속도 제한 변경 알림"), tr("속도 제한이 변경될 때마다 알림을 트리거합니다."), ""},
    {"UseVienna", tr("Vienna 속도 제한 표지판 사용"), tr("MUTCD(US) 대신 Vienna(EU) 속도 제한 스타일 표지판 사용."), ""},

    {"VisionTurnControl", tr("화면 턴 속도 조절"), tr("도로에서 감지된 곡선에서 속도를 줄입니다."), "../frogpilot/assets/toggle_icons/icon_vtc.png"},
    {"DisableVTSCSmoothing", tr("화면 턴 속도 조절 UI Smoothing 비활성화"), tr("onroad UI에서 요청된 속도에 대한 Smoothing을 비활성화합니다."), ""},
    {"CurveSensitivity", tr("곡선 감지 감도"), tr("곡선 감지 감도를 설정합니다. 값이 높을수록 반응이 빠르고, 값이 낮을수록 반응은 부드럽지만 반응은 느립니다."), ""},
    {"TurnAggressiveness", tr("회전 속도 공격성"), tr("회전 속도 공격성을 설정합니다. 값이 높을수록 회전 속도가 빨라지고, 값이 낮을수록 회전이 부드럽습니다."), ""},
  };

  for (const auto &[param, title, desc, icon] : controlToggles) {
    AbstractControl *toggle;

    if (param == "AlwaysOnLateral") {
      FrogPilotParamManageControl *aolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(aolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(aolKeys.find(key.c_str()) != aolKeys.end());
        }
      });
      toggle = aolToggle;
    } else if (param == "PauseAOLOnBrake") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));

    } else if (param == "ConditionalExperimental") {
      FrogPilotParamManageControl *conditionalExperimentalToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(conditionalExperimentalToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        conditionalSpeedsImperial->setVisible(!isMetric);
        conditionalSpeedsMetric->setVisible(isMetric);
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end());
        }
      });
      toggle = conditionalExperimentalToggle;
    } else if (param == "CECurves") {
      FrogPilotParamValueControl *CESpeedImperial = new FrogPilotParamValueControl("CESpeed", tr("Below"), tr("Switch to 'Experimental Mode' below this speed when not following a lead vehicle."), "", 0, 99,
                                                                                   std::map<int, QString>(), this, false, tr(" mph"));
      FrogPilotParamValueControl *CESpeedLeadImperial = new FrogPilotParamValueControl("CESpeedLead", tr("  w/Lead"), tr("Switch to 'Experimental Mode' below this speed when following a lead vehicle."), "", 0, 99,
                                                                                       std::map<int, QString>(), this, false, tr(" mph"));
      conditionalSpeedsImperial = new FrogPilotDualParamControl(CESpeedImperial, CESpeedLeadImperial, this);
      addItem(conditionalSpeedsImperial);

      FrogPilotParamValueControl *CESpeedMetric = new FrogPilotParamValueControl("CESpeed", tr("Below"), tr("Switch to 'Experimental Mode' below this speed in absence of a lead vehicle."), "", 0, 150,
                                                                                 std::map<int, QString>(), this, false, tr(" kph"));
      FrogPilotParamValueControl *CESpeedLeadMetric = new FrogPilotParamValueControl("CESpeedLead", tr("  w/Lead"), tr("Switch to 'Experimental Mode' below this speed when following a lead vehicle."), "", 0, 150,
                                                                                     std::map<int, QString>(), this, false, tr(" kph"));
      conditionalSpeedsMetric = new FrogPilotDualParamControl(CESpeedMetric, CESpeedLeadMetric, this);
      addItem(conditionalSpeedsMetric);

      std::vector<QString> curveToggles{"CECurvesLead"};
      std::vector<QString> curveToggleNames{tr("With Lead")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, curveToggles, curveToggleNames);
    } else if (param == "CENavigation") {
      std::vector<QString> navigationToggles{"CENavigationIntersections", "CENavigationTurns", "CENavigationLead"};
      std::vector<QString> navigationToggleNames{tr("Intersections"), tr("Turns"), tr("With Lead")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, navigationToggles, navigationToggleNames);
    } else if (param == "CEStopLights") {
      std::vector<QString> stopLightToggles{"CEStopLightsLead"};
      std::vector<QString> stopLightToggleNames{tr("With Lead")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, stopLightToggles, stopLightToggleNames);

    } else if (param == "DeviceManagement") {
      FrogPilotParamManageControl *deviceManagementToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(deviceManagementToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(deviceManagementKeys.find(key.c_str()) != deviceManagementKeys.end());
        }
      });
      toggle = deviceManagementToggle;
    } else if (param == "DeviceShutdown") {
      std::map<int, QString> shutdownLabels;
      for (int i = 0; i <= 33; ++i) {
        shutdownLabels[i] = i == 0 ? tr("5 mins") : i <= 3 ? QString::number(i * 15) + tr(" mins") : QString::number(i - 3) + (i == 4 ? tr(" hour") : tr(" hours"));
      }
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 33, shutdownLabels, this, false);
    } else if (param == "NoUploads") {
      std::vector<QString> uploadsToggles{"DisableOnroadUploads"};
      std::vector<QString> uploadsToggleNames{tr("Only Onroad")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, uploadsToggles, uploadsToggleNames);
    } else if (param == "LowVoltageShutdown") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 11.8, 12.5, std::map<int, QString>(), this, false, tr(" volts"), 1, 0.01);

    } else if (param == "DrivingPersonalities") {
      FrogPilotParamManageControl *drivingPersonalitiesToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(drivingPersonalitiesToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(drivingPersonalityKeys.find(key.c_str()) != drivingPersonalityKeys.end());
        }
      });
      toggle = drivingPersonalitiesToggle;
    } else if (param == "CustomPersonalities") {
      FrogPilotParamManageControl *customPersonalitiesToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customPersonalitiesToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        customPersonalitiesOpen = true;
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customdrivingPersonalityKeys.find(key.c_str()) != customdrivingPersonalityKeys.end());
          openSubParentToggle();
        }
      });

      personalitiesInfoBtn = new ButtonControl(tr("What Do All These Do?"), tr("VIEW"), tr("Learn what all the values in 'Custom Personality Profiles' do on openpilot's driving behaviors."));
      connect(personalitiesInfoBtn, &ButtonControl::clicked, [=]() {
        const std::string txt = util::read_file("../frogpilot/ui/qt/offroad/personalities_info.txt");
        ConfirmationDialog::rich(QString::fromStdString(txt), this);
      });
      addItem(personalitiesInfoBtn);

      toggle = customPersonalitiesToggle;
    } else if (param == "ResetTrafficPersonality" || param == "ResetAggressivePersonality" || param == "ResetStandardPersonality" || param == "ResetRelaxedPersonality") {
      std::vector<QString> personalityOptions{tr("Reset")};
      FrogPilotButtonsControl *profileBtn = new FrogPilotButtonsControl(title, desc, icon, personalityOptions);
      toggle = profileBtn;
    } else if (param == "TrafficPersonalityProfile") {
      FrogPilotParamManageControl *trafficPersonalityToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(trafficPersonalityToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(trafficPersonalityKeys.find(key.c_str()) != trafficPersonalityKeys.end());
        }
        openSubSubParentToggle();
        personalitiesInfoBtn->setVisible(true);
      });
      toggle = trafficPersonalityToggle;
    } else if (param == "AggressivePersonalityProfile") {
      FrogPilotParamManageControl *aggressivePersonalityToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(aggressivePersonalityToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(aggressivePersonalityKeys.find(key.c_str()) != aggressivePersonalityKeys.end());
        }
        openSubSubParentToggle();
        personalitiesInfoBtn->setVisible(true);
      });
      toggle = aggressivePersonalityToggle;
    } else if (param == "StandardPersonalityProfile") {
      FrogPilotParamManageControl *standardPersonalityToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(standardPersonalityToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(standardPersonalityKeys.find(key.c_str()) != standardPersonalityKeys.end());
        }
        openSubSubParentToggle();
        personalitiesInfoBtn->setVisible(true);
      });
      toggle = standardPersonalityToggle;
    } else if (param == "RelaxedPersonalityProfile") {
      FrogPilotParamManageControl *relaxedPersonalityToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(relaxedPersonalityToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(relaxedPersonalityKeys.find(key.c_str()) != relaxedPersonalityKeys.end());
        }
        openSubSubParentToggle();
        personalitiesInfoBtn->setVisible(true);
      });
      toggle = relaxedPersonalityToggle;
    } else if (trafficPersonalityKeys.find(param) != trafficPersonalityKeys.end() ||
               aggressivePersonalityKeys.find(param) != aggressivePersonalityKeys.end() ||
               standardPersonalityKeys.find(param) != standardPersonalityKeys.end() ||
               relaxedPersonalityKeys.find(param) != relaxedPersonalityKeys.end()) {
      if (param == "TrafficFollow" || param == "AggressiveFollow" || param == "StandardFollow" || param == "RelaxedFollow") {
        if (param == "TrafficFollow") {
          toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0.5, 5, std::map<int, QString>(), this, false, tr(" seconds"), 1, 0.01);
        } else {
          toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 5, std::map<int, QString>(), this, false, tr(" seconds"), 1, 0.01);
        }
      } else {
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 500, std::map<int, QString>(), this, false, "%");
      }
    } else if (param == "OnroadDistanceButton") {
      std::vector<QString> onroadDistanceToggles{"KaofuiIcons"};
      std::vector<QString> onroadDistanceToggleNames{tr("Kaofui's Icons")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, onroadDistanceToggles, onroadDistanceToggleNames);

    } else if (param == "ExperimentalModeActivation") {
      FrogPilotParamManageControl *experimentalModeActivationToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(experimentalModeActivationToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(experimentalModeActivationKeys.find(key.c_str()) != experimentalModeActivationKeys.end());
        }
      });
      toggle = experimentalModeActivationToggle;

    } else if (param == "LateralTune") {
      FrogPilotParamManageControl *lateralTuneToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(lateralTuneToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedLateralTuneKeys = lateralTuneKeys;

          if (hasAutoTune || params.getBool("LateralTune") && params.getBool("NNFF")) {
            modifiedLateralTuneKeys.erase("ForceAutoTune");
          }

          if (hasCommaNNFFSupport) {
            modifiedLateralTuneKeys.erase("NNFF");
            modifiedLateralTuneKeys.erase("NNFFLite");
          } else if (hasNNFFLog) {
            modifiedLateralTuneKeys.erase("NNFFLite");
          } else {
            modifiedLateralTuneKeys.erase("NNFF");
          }

          toggle->setVisible(modifiedLateralTuneKeys.find(key.c_str()) != modifiedLateralTuneKeys.end());
        }
      });
      toggle = lateralTuneToggle;
    } else if (param == "SteerRatio") {
      std::vector<QString> steerRatioToggles{"ResetSteerRatio"};
      std::vector<QString> steerRatioToggleNames{"Reset"};
      toggle = new FrogPilotParamValueToggleControl(param, title, desc, icon, steerRatioStock * 0.75, steerRatioStock * 1.25, std::map<int, QString>(), this, false, "", 1, 0.01, steerRatioToggles, steerRatioToggleNames);

    } else if (param == "LongitudinalTune") {
      FrogPilotParamManageControl *longitudinalTuneToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(longitudinalTuneToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedLongitudinalTuneKeys = longitudinalTuneKeys;

          if (params.get("Model") == "radical-turtle") {
            modifiedLongitudinalTuneKeys.erase("LeadDetectionThreshold");
          }

          toggle->setVisible(modifiedLongitudinalTuneKeys.find(key.c_str()) != modifiedLongitudinalTuneKeys.end());
        }
      });
      toggle = longitudinalTuneToggle;
    } else if (param == "AccelerationProfile") {
      std::vector<QString> profileOptions{tr("Standard"), tr("Eco"), tr("Sport"), tr("Sport+")};
      FrogPilotButtonParamControl *profileSelection = new FrogPilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;

      QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
        if (id == 3) {
          FrogPilotConfirmationDialog::toggleAlert(tr("WARNING: This maxes out openpilot's acceleration from 2.0 m/s to 4.0 m/s and may cause oscillations when accelerating!"),
          tr("I understand the risks."), this);
        }
      });
    } else if (param == "AggressiveAcceleration") {
      std::vector<QString> accelerationToggles{"AggressiveAccelerationExperimental"};
      std::vector<QString> accelerationToggleNames{tr("Experimental")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, accelerationToggles, accelerationToggleNames);
      QObject::connect(static_cast<FrogPilotParamToggleControl*>(toggle), &FrogPilotParamToggleControl::buttonClicked, [this](bool checked) {
        if (checked) {
          FrogPilotConfirmationDialog::toggleAlert(
          tr("WARNING: This is very experimental and may cause the car to not brake or stop safely! Please report any issues in the FrogPilot Discord!"),
          tr("I understand the risks."), this);
        }
      });
    } else if (param == "DecelerationProfile") {
      std::vector<QString> profileOptions{tr("Standard"), tr("Eco"), tr("Sport")};
      FrogPilotButtonParamControl *profileSelection = new FrogPilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;
    } else if (param == "StoppingDistance") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, std::map<int, QString>(), this, false, tr(" feet"));
    } else if (param == "LeadDetectionThreshold") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 99, std::map<int, QString>(), this, false, "%");
    } else if (param == "SmoothBraking") {
      std::vector<QString> brakingToggles{"SmoothBrakingJerk", "SmoothBrakingFarLead"};
      std::vector<QString> brakingToggleNames{tr("Apply to Jerk"), tr("Far Lead Offset")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, brakingToggles, brakingToggleNames);
      QObject::connect(static_cast<FrogPilotParamToggleControl*>(toggle), &FrogPilotParamToggleControl::buttonClicked, [this](bool checked) {
        if (checked) {
          FrogPilotConfirmationDialog::toggleAlert(
          tr("WARNING: This is very experimental and may cause the car to not brake or stop safely! Please report any issues in the FrogPilot Discord!"),
          tr("I understand the risks."), this);
        }
      });

    } else if (param == "MTSCEnabled") {
      FrogPilotParamManageControl *mtscToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(mtscToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(mtscKeys.find(key.c_str()) != mtscKeys.end());
        }
      });
      toggle = mtscToggle;
    } else if (param == "MTSCAggressiveness") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

    } else if (param == "ModelSelector") {
      FrogPilotParamManageControl *modelsToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(modelsToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(false);
        }

        deleteModelBtn->setVisible(true);
        downloadModelBtn->setVisible(true);
        selectModelBtn->setVisible(true);
      });
      toggle = modelsToggle;

      QDir modelDir("/data/models/");

      deleteModelBtn = new ButtonControl(tr("모델 삭제"), tr("삭제"), "");
      QObject::connect(deleteModelBtn, &ButtonControl::clicked, [=]() {
        std::string currentModel = params.get("Model") + ".thneed";

        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QStringList existingModelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);
        QMap<QString, QString> labelToFileMap;
        QStringList deletableModelLabels;
        for (int i = 0; i < availableModels.size(); ++i) {
          QString modelFileName = availableModels[i] + ".thneed";
          if (existingModelFiles.contains(modelFileName) && modelFileName != QString::fromStdString(currentModel)) {
            QString readableName = modelLabels[i];
            deletableModelLabels.append(readableName);
            labelToFileMap[readableName] = modelFileName;
          }
        }

        QString selectedModel = MultiOptionDialog::getSelection(tr("삭제할 모델을 선택"), deletableModelLabels, "", this);
        if (!selectedModel.isEmpty() && ConfirmationDialog::confirm(tr("이 모델을 삭제 하시겠습니까?"), tr("삭제"), this)) {
          std::thread([=]() {
            deleteModelBtn->setValue(tr("삭제중..."));

            deleteModelBtn->setEnabled(false);
            downloadModelBtn->setEnabled(false);
            selectModelBtn->setEnabled(false);

            QString modelToDelete = labelToFileMap[selectedModel];

            QFile::remove(modelDir.absoluteFilePath(modelToDelete));

            deleteModelBtn->setEnabled(true);
            downloadModelBtn->setEnabled(true);
            selectModelBtn->setEnabled(true);

            deleteModelBtn->setValue(tr("삭제!"));
            std::this_thread::sleep_for(std::chrono::seconds(3));
            deleteModelBtn->setValue("");
          }).detach();
        }
      });
      addItem(deleteModelBtn);

      downloadModelBtn = new ButtonControl(tr("모델 다운로드"), tr("다운로드"), "");
      QObject::connect(downloadModelBtn, &ButtonControl::clicked, [=]() {
        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QMap<QString, QString> labelToModelMap;
        QStringList downloadableModelLabels;
        QStringList existingModelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);
        for (int i = 0; i < availableModels.size(); ++i) {
          QString modelFileName = availableModels.at(i) + ".thneed";
          if (!existingModelFiles.contains(modelFileName)) {
            QString readableName = modelLabels.at(i);
            if (!readableName.contains("(Default)")) {
              downloadableModelLabels.append(readableName);
              labelToModelMap.insert(readableName, availableModels.at(i));
            }
          }
        }

        QString modelToDownload = MultiOptionDialog::getSelection(tr("다운로드할 모델을 선택하세요"), downloadableModelLabels, "", this);
        if (!modelToDownload.isEmpty()) {
          QString selectedModelValue = labelToModelMap.value(modelToDownload);
          paramsMemory.put("ModelToDownload", selectedModelValue.toStdString());

          deleteModelBtn->setEnabled(false);
          downloadModelBtn->setEnabled(false);
          selectModelBtn->setEnabled(false);

          QTimer *failureTimer = new QTimer(this);
          failureTimer->setSingleShot(true);

          QTimer *progressTimer = new QTimer(this);
          progressTimer->setInterval(100);

          connect(failureTimer, &QTimer::timeout, this, [=]() {
            deleteModelBtn->setEnabled(true);
            downloadModelBtn->setEnabled(true);
            selectModelBtn->setEnabled(true);

            downloadModelBtn->setValue(tr("다운로드 실패..."));
            paramsMemory.remove("ModelDownloadProgress");
            paramsMemory.remove("ModelToDownload");

            progressTimer->stop();
            progressTimer->deleteLater();

            QTimer::singleShot(3000, this, [this]() {
              downloadModelBtn->setValue("");
            });
          });

          connect(progressTimer, &QTimer::timeout, this, [=]() mutable {
            static int lastProgress = -1;
            int progress = paramsMemory.getInt("ModelDownloadProgress");

            if (progress == lastProgress) {
              if (!failureTimer->isActive()) {
                failureTimer->start(30000);
              }
            } else {
              lastProgress = progress;
              downloadModelBtn->setValue(QString::number(progress) + "%");
              failureTimer->stop();

              if (progress == 100) {
                deleteModelBtn->setEnabled(true);
                downloadModelBtn->setEnabled(true);
                selectModelBtn->setEnabled(true);

                downloadModelBtn->setValue(tr("다운로드!"));
                paramsMemory.remove("ModelDownloadProgress");
                paramsMemory.remove("ModelToDownload");

                progressTimer->stop();
                progressTimer->deleteLater();

                QTimer::singleShot(3000, this, [this]() {
                  if (paramsMemory.get("ModelDownloadProgress").empty()) {
                    downloadModelBtn->setValue("");
                  }
                });
              }
            }
          });
          progressTimer->start();
        }
      });
      addItem(downloadModelBtn);

      selectModelBtn = new ButtonControl(tr("모델 선택"), tr("선택"), "");
      QObject::connect(selectModelBtn, &ButtonControl::clicked, [=]() {
        QStringList availableModels = QString::fromStdString(params.get("AvailableModels")).split(",");
        QStringList modelLabels = QString::fromStdString(params.get("AvailableModelsNames")).split(",");

        QStringList modelFiles = modelDir.entryList({"*.thneed"}, QDir::Files);
        QSet<QString> modelFilesBaseNames;
        for (const QString &modelFile : modelFiles) {
          modelFilesBaseNames.insert(modelFile.section('.', 0, 0));
        }

        QStringList selectableModelLabels;
        for (int i = 0; i < availableModels.size(); ++i) {
          if (modelFilesBaseNames.contains(availableModels[i]) || modelLabels[i].contains("(Default)")) {
            selectableModelLabels.append(modelLabels[i]);
          }
        }

        QString modelToSelect = MultiOptionDialog::getSelection(tr("Select a model - 🗺️ = Navigation | 📡 = Radar | 👀 = VOACC"), selectableModelLabels, "", this);
        if (!modelToSelect.isEmpty()) {
          selectModelBtn->setValue(modelToSelect);

          int modelIndex = modelLabels.indexOf(modelToSelect);
          if (modelIndex != -1) {
            QString selectedModel = availableModels.at(modelIndex);
            params.putNonBlocking("Model", selectedModel.toStdString());
            params.putNonBlocking("ModelName", modelToSelect.toStdString());
          }

          if (FrogPilotConfirmationDialog::yesorno(tr("새로 선택한 모델에 대한 새로운 캘리브레이션을 시작하시겠습니까?"), this)) {
            params.remove("CalibrationParams");
            params.remove("LiveTorqueParameters");
          }

          if (started) {
            if (FrogPilotConfirmationDialog::toggle(tr("적용하려면 재부팅이 필요합니다."), tr("지금 재시작"), this)) {
              Hardware::reboot();
            }
          }
        }
      });
      addItem(selectModelBtn);
      selectModelBtn->setValue(QString::fromStdString(params.get("ModelName")));

    } else if (param == "QOLControls") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedQolKeys = qolKeys;

          if (!hasPCMCruise) {
            modifiedQolKeys.erase("ReverseCruise");
          } else {
            modifiedQolKeys.erase("CustomCruise");
            modifiedQolKeys.erase("CustomCruiseLong");
            modifiedQolKeys.erase("SetSpeedOffset");
          }

          if (!isToyota && !isGM && !isHKGCanFd) {
            modifiedQolKeys.erase("MapGears");
          }

          toggle->setVisible(modifiedQolKeys.find(key.c_str()) != modifiedQolKeys.end());
        }
      });
      toggle = qolToggle;
    } else if (param == "CustomCruise") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "CustomCruiseLong") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "MapGears") {
      std::vector<QString> mapGearsToggles{"MapAcceleration", "MapDeceleration"};
      std::vector<QString> mapGearsToggleNames{tr("Acceleration"), tr("Deceleration")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, mapGearsToggles, mapGearsToggleNames);
    } else if (param == "PauseLateralSpeed") {
      std::vector<QString> pauseLateralToggles{"PauseLateralOnSignal"};
      std::vector<QString> pauseLateralToggleNames{"Turn Signal Only"};
      toggle = new FrogPilotParamValueToggleControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"), 1, 1, pauseLateralToggles, pauseLateralToggleNames);
    } else if (param == "PauseLateralOnSignal") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "ReverseCruise") {
      std::vector<QString> reverseCruiseToggles{"ReverseCruiseUI"};
      std::vector<QString> reverseCruiseNames{tr("Control Via UI")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, reverseCruiseToggles, reverseCruiseNames);
    } else if (param == "SetSpeedOffset") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));

    } else if (param == "LaneChangeCustomizations") {
      FrogPilotParamManageControl *laneChangeToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(laneChangeToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(laneChangeKeys.find(key.c_str()) != laneChangeKeys.end());
        }
      });
      toggle = laneChangeToggle;
    } else if (param == "MinimumLaneChangeSpeed") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "LaneChangeTime") {
      std::map<int, QString> laneChangeTimeLabels;
      for (int i = 0; i <= 10; ++i) {
        laneChangeTimeLabels[i] = i == 0 ? "Instant" : QString::number(i / 2.0) + " seconds";
      }
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, laneChangeTimeLabels, this, false);
    } else if (param == "LaneDetectionWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, " feet", 10);

    } else if (param == "SpeedLimitController") {
      FrogPilotParamManageControl *speedLimitControllerToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(speedLimitControllerToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        slcOpen = true;
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end());
        }
      });
      toggle = speedLimitControllerToggle;
    } else if (param == "SLCControls") {
      FrogPilotParamManageControl *manageSLCControlsToggle = new FrogPilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCControlsToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerControlsKeys.find(key.c_str()) != speedLimitControllerControlsKeys.end());
          openSubParentToggle();
        }
      });
      toggle = manageSLCControlsToggle;
    } else if (param == "SLCQOL") {
      FrogPilotParamManageControl *manageSLCQOLToggle = new FrogPilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCQOLToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          std::set<QString> modifiedSpeedLimitControllerQOLKeys = speedLimitControllerQOLKeys;

          if (hasPCMCruise) {
            modifiedSpeedLimitControllerQOLKeys.erase("SetSpeedLimit");
          }

          if (!isToyota) {
            modifiedSpeedLimitControllerQOLKeys.erase("ForceMPHDashboard");
          }

          toggle->setVisible(modifiedSpeedLimitControllerQOLKeys.find(key.c_str()) != modifiedSpeedLimitControllerQOLKeys.end());
          openSubParentToggle();
        }
      });
      toggle = manageSLCQOLToggle;
    } else if (param == "SLCConfirmation") {
      std::vector<QString> slcConfirmationToggles{"SLCConfirmationLower", "SLCConfirmationHigher"};
      std::vector<QString> slcConfirmationNames{tr("Lower Limits"), tr("Higher Limits")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, slcConfirmationToggles, slcConfirmationNames);
    } else if (param == "SLCLookaheadHigher" || param == "SLCLookaheadLower") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 60, std::map<int, QString>(), this, false, " seconds");
    } else if (param == "SLCVisuals") {
      FrogPilotParamManageControl *manageSLCVisualsToggle = new FrogPilotParamManageControl(param, title, desc, icon, this, true);
      QObject::connect(manageSLCVisualsToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerVisualsKeys.find(key.c_str()) != speedLimitControllerVisualsKeys.end());
          openSubParentToggle();
        }
      });
      toggle = manageSLCVisualsToggle;
    } else if (param == "Offset1" || param == "Offset2" || param == "Offset3" || param == "Offset4") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, -99, 99, std::map<int, QString>(), this, false, tr(" mph"));
    } else if (param == "ShowSLCOffset") {
      std::vector<QString> slcOffsetToggles{"ShowSLCOffsetUI"};
      std::vector<QString> slcOffsetToggleNames{tr("Control Via UI")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, slcOffsetToggles, slcOffsetToggleNames);
    } else if (param == "SLCFallback") {
      std::vector<QString> fallbackOptions{tr("Set Speed"), tr("Experimental Mode"), tr("Previous Limit")};
      FrogPilotButtonParamControl *fallbackSelection = new FrogPilotButtonParamControl(param, title, desc, icon, fallbackOptions);
      toggle = fallbackSelection;
    } else if (param == "SLCOverride") {
      std::vector<QString> overrideOptions{tr("None"), tr("Manual Set Speed"), tr("Set Speed")};
      FrogPilotButtonParamControl *overrideSelection = new FrogPilotButtonParamControl(param, title, desc, icon, overrideOptions);
      toggle = overrideSelection;
    } else if (param == "SLCPriority") {
      ButtonControl *slcPriorityButton = new ButtonControl(title, tr("SELECT"), desc);
      QStringList primaryPriorities = {tr("None"), tr("Dashboard"), tr("Navigation"), tr("Offline Maps"), tr("Highest"), tr("Lowest")};
      QStringList secondaryTertiaryPriorities = {tr("None"), tr("Dashboard"), tr("Navigation"), tr("Offline Maps")};
      QStringList priorityPrompts = {tr("Select your primary priority"), tr("Select your secondary priority"), tr("Select your tertiary priority")};

      QObject::connect(slcPriorityButton, &ButtonControl::clicked, [=]() {
        QStringList selectedPriorities;

        for (int i = 1; i <= 3; ++i) {
          QStringList currentPriorities = (i == 1) ? primaryPriorities : secondaryTertiaryPriorities;
          QStringList prioritiesToDisplay = currentPriorities;
          for (const auto &selectedPriority : qAsConst(selectedPriorities)) {
            prioritiesToDisplay.removeAll(selectedPriority);
          }

          if (!hasDashSpeedLimits) {
            prioritiesToDisplay.removeAll(tr("Dashboard"));
          }

          if (prioritiesToDisplay.size() == 1 && prioritiesToDisplay.contains(tr("None"))) {
            break;
          }

          QString priorityKey = QString("SLCPriority%1").arg(i);
          QString selection = MultiOptionDialog::getSelection(priorityPrompts[i - 1], prioritiesToDisplay, "", this);

          if (selection.isEmpty()) break;

          params.putNonBlocking(priorityKey.toStdString(), selection.toStdString());
          selectedPriorities.append(selection);

          if (selection == tr("Lowest") || selection == tr("Highest") || selection == tr("None")) break;

          updateFrogPilotToggles();
        }

        selectedPriorities.removeAll(tr("None"));
        slcPriorityButton->setValue(selectedPriorities.join(", "));
      });

      QStringList initialPriorities;
      for (int i = 1; i <= 3; ++i) {
        QString priorityKey = QString("SLCPriority%1").arg(i);
        QString priority = QString::fromStdString(params.get(priorityKey.toStdString()));

        if (!priority.isEmpty() && primaryPriorities.contains(priority) && priority != tr("None")) {
          initialPriorities.append(priority);
        }
      }
      slcPriorityButton->setValue(initialPriorities.join(", "));
      toggle = slcPriorityButton;

    } else if (param == "VisionTurnControl") {
      FrogPilotParamManageControl *visionTurnControlToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(visionTurnControlToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end());
        }
      });
      toggle = visionTurnControlToggle;
    } else if (param == "CurveSensitivity" || param == "TurnAggressiveness") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(static_cast<ToggleControl*>(toggle), &ToggleControl::toggleFlipped, &updateFrogPilotToggles);
    QObject::connect(static_cast<FrogPilotParamValueControl*>(toggle), &FrogPilotParamValueControl::valueChanged, &updateFrogPilotToggles);

    ParamWatcher *param_watcher = new ParamWatcher(this);
    param_watcher->addParam("CESpeed");
    param_watcher->addParam("CESpeedLead");

    QObject::connect(param_watcher, &ParamWatcher::paramChanged, [=](const QString &param_name, const QString &param_value) {
      updateFrogPilotToggles();
    });

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });

    QObject::connect(static_cast<FrogPilotParamManageControl*>(toggle), &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
      update();
    });
  }

  QObject::connect(static_cast<ToggleControl*>(toggles["IncreaseThermalLimits"]), &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("IncreaseThermalLimits")) {
      FrogPilotConfirmationDialog::toggleAlert(
        tr("WARNING: This can cause premature wear or damage by running the device over comma's recommended temperature limits!"),
        tr("I understand the risks."), this);
    }
  });

  QObject::connect(static_cast<ToggleControl*>(toggles["NoLogging"]), &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("NoLogging")) {
      FrogPilotConfirmationDialog::toggleAlert(
        tr("WARNING: This will prevent your drives from being recorded and the data will be unobtainable!"),
        tr("I understand the risks."), this);
    }
  });

  QObject::connect(static_cast<ToggleControl*>(toggles["NoUploads"]), &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("NoUploads")) {
      FrogPilotConfirmationDialog::toggleAlert(
        tr("WARNING: This will prevent your drives from appearing on comma connect which may impact debugging and support!"),
        tr("I understand the risks."), this);
    }
  });

  QObject::connect(static_cast<ToggleControl*>(toggles["TrafficMode"]), &ToggleControl::toggleFlipped, [this]() {
    if (params.getBool("TrafficMode")) {
      FrogPilotConfirmationDialog::toggleAlert(
        tr("To activate 'Traffic Mode' you hold down the 'distance' button on your steering wheel for 2.5 seconds."),
        tr("Sounds good!"), this);
    }
  });

  std::set<QString> rebootKeys = {"AlwaysOnLateral", "NNFF", "NNFFLite"};
  for (const QString &key : rebootKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key.toStdString().c_str()]), &ToggleControl::toggleFlipped, [this]() {
      if (started) {
        if (FrogPilotConfirmationDialog::toggle(tr("Reboot required to take effect."), tr("Reboot Now"), this)) {
          Hardware::reboot();
        }
      }
    });
  }

  FrogPilotParamValueControl *trafficFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["TrafficFollow"]);
  FrogPilotParamValueControl *trafficAccelerationoggle = static_cast<FrogPilotParamValueControl*>(toggles["TrafficJerkAcceleration"]);
  FrogPilotParamValueControl *trafficSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["TrafficJerkSpeed"]);
  FrogPilotButtonsControl *trafficResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetTrafficPersonality"]);

  QObject::connect(trafficResetButton, &FrogPilotButtonsControl::buttonClicked, this, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the 'Traffic Mode' personality?"), this)) {
      params.putFloat("TrafficFollow", 0.5);
      params.putFloat("TrafficJerkAcceleration", 50);
      params.putFloat("TrafficJerkSpeed", 75);
      trafficFollowToggle->refresh();
      trafficAccelerationoggle->refresh();
      trafficSpeedToggle->refresh();
      updateFrogPilotToggles();
    }
  });

  FrogPilotParamValueControl *aggressiveFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveFollow"]);
  FrogPilotParamValueControl *aggressiveAccelerationoggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkAcceleration"]);
  FrogPilotParamValueControl *aggressiveSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkSpeed"]);
  FrogPilotButtonsControl *aggressiveResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetAggressivePersonality"]);

  QObject::connect(aggressiveResetButton, &FrogPilotButtonsControl::buttonClicked, this, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the 'Aggressive' personality?"), this)) {
      params.putFloat("AggressiveFollow", 1.25);
      params.putFloat("AggressiveJerkAcceleration", 50);
      params.putFloat("AggressiveJerkSpeed", 50);
      aggressiveFollowToggle->refresh();
      aggressiveAccelerationoggle->refresh();
      aggressiveSpeedToggle->refresh();
      updateFrogPilotToggles();
    }
  });

  FrogPilotParamValueControl *standardFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardFollow"]);
  FrogPilotParamValueControl *standardAccelerationoggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkAcceleration"]);
  FrogPilotParamValueControl *standardSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkSpeed"]);
  FrogPilotButtonsControl *standardResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetStandardPersonality"]);

  QObject::connect(standardResetButton, &FrogPilotButtonsControl::buttonClicked, this, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the 'Standard' personality?"), this)) {
      params.putFloat("StandardFollow", 1.45);
      params.putFloat("StandardJerkAcceleration", 100);
      params.putFloat("StandardJerkSpeed", 100);
      standardFollowToggle->refresh();
      standardAccelerationoggle->refresh();
      standardSpeedToggle->refresh();
      updateFrogPilotToggles();
    }
  });

  FrogPilotParamValueControl *relaxedFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedFollow"]);
  FrogPilotParamValueControl *relaxedAccelerationoggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkAcceleration"]);
  FrogPilotParamValueControl *relaxedSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkSpeed"]);
  FrogPilotButtonsControl *relaxedResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetRelaxedPersonality"]);

  QObject::connect(relaxedResetButton, &FrogPilotButtonsControl::buttonClicked, this, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the 'Relaxed' personality?"), this)) {
      params.putFloat("RelaxedFollow", 1.75);
      params.putFloat("RelaxedJerkAcceleration", 100);
      params.putFloat("RelaxedJerkSpeed", 100);
      relaxedFollowToggle->refresh();
      relaxedAccelerationoggle->refresh();
      relaxedSpeedToggle->refresh();
      updateFrogPilotToggles();
    }
  });

  modelManagerToggle = static_cast<FrogPilotParamManageControl*>(toggles["ModelSelector"]);
  steerRatioToggle = static_cast<FrogPilotParamValueToggleControl*>(toggles["SteerRatio"]);

  QObject::connect(steerRatioToggle, &FrogPilotParamValueToggleControl::buttonClicked, this, [this]() {
    params.putFloat("SteerRatio", steerRatioStock);
    params.putBool("ResetSteerRatio", false);
    steerRatioToggle->refresh();
    updateFrogPilotToggles();
  });

  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotControlsPanel::hideToggles);
  QObject::connect(parent, &SettingsWindow::closeSubParentToggle, this, &FrogPilotControlsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeSubSubParentToggle, this, &FrogPilotControlsPanel::hideSubSubToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotControlsPanel::updateMetric);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &FrogPilotControlsPanel::updateCarToggles);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotControlsPanel::updateState);

  updateMetric();
}

void FrogPilotControlsPanel::showEvent(QShowEvent *event, const UIState &s) {
  hasOpenpilotLongitudinal = hasOpenpilotLongitudinal && !params.getBool("DisableOpenpilotLongitudinal");

  downloadModelBtn->setEnabled(s.scene.online);
}

void FrogPilotControlsPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  started = s.scene.started;

  modelManagerToggle->setEnabled(!s.scene.started || s.scene.parked);
}

void FrogPilotControlsPanel::updateCarToggles() {
  auto carParams = params.get("CarParamsPersistent");
  if (!carParams.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(carParams.data(), carParams.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();
    auto carFingerprint = CP.getCarFingerprint();
    auto carName = CP.getCarName();
    auto safetyConfigs = CP.getSafetyConfigs();
    auto safetyModel = safetyConfigs[0].getSafetyModel();

    hasAutoTune = (carName == "hyundai" || carName == "toyota") && CP.getLateralTuning().which() == cereal::CarParams::LateralTuning::TORQUE;
    uiState()->scene.has_auto_tune = hasAutoTune;
    hasCommaNNFFSupport = checkCommaNNFFSupport(carFingerprint);
    hasDashSpeedLimits = carName == "hyundai" || carName == "toyota";
    hasNNFFLog = checkNNFFLogFileExists(carFingerprint);
    hasOpenpilotLongitudinal = CP.getOpenpilotLongitudinalControl() && !params.getBool("DisableOpenpilotLongitudinal");
    hasPCMCruise = CP.getPcmCruise();
    isGM = carName == "gm";
    isHKGCanFd = (carName == "hyundai") && (safetyModel == cereal::CarParams::SafetyModel::HYUNDAI_CANFD);
    isToyota = carName == "toyota";
    steerRatioStock = CP.getSteerRatio();

    steerRatioToggle->setTitle(QString(tr("Steer Ratio (Default: %1)")).arg(QString::number(steerRatioStock, 'f', 2)));
    steerRatioToggle->updateControl(steerRatioStock * 0.75, steerRatioStock * 1.25, "", 0.01);
    steerRatioToggle->refresh();
  } else {
    hasAutoTune = false;
    hasCommaNNFFSupport = false;
    hasDashSpeedLimits = true;
    hasNNFFLog = true;
    hasOpenpilotLongitudinal = true;
    hasPCMCruise = true;
    isGM = true;
    isHKGCanFd = true;
    isToyota = true;
  }

  hideToggles();
}

void FrogPilotControlsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    double speedConversion = isMetric ? MILE_TO_KM : KM_TO_MILE;

    params.putIntNonBlocking("CESpeed", std::nearbyint(params.getInt("CESpeed") * speedConversion));
    params.putIntNonBlocking("CESpeedLead", std::nearbyint(params.getInt("CESpeedLead") * speedConversion));
    params.putIntNonBlocking("CustomCruise", std::nearbyint(params.getInt("CustomCruise") * speedConversion));
    params.putIntNonBlocking("CustomCruiseLong", std::nearbyint(params.getInt("CustomCruiseLong") * speedConversion));
    params.putIntNonBlocking("LaneDetectionWidth", std::nearbyint(params.getInt("LaneDetectionWidth") * distanceConversion));
    params.putIntNonBlocking("MinimumLaneChangeSpeed", std::nearbyint(params.getInt("MinimumLaneChangeSpeed") * speedConversion));
    params.putIntNonBlocking("Offset1", std::nearbyint(params.getInt("Offset1") * speedConversion));
    params.putIntNonBlocking("Offset2", std::nearbyint(params.getInt("Offset2") * speedConversion));
    params.putIntNonBlocking("Offset3", std::nearbyint(params.getInt("Offset3") * speedConversion));
    params.putIntNonBlocking("Offset4", std::nearbyint(params.getInt("Offset4") * speedConversion));
    params.putIntNonBlocking("PauseAOLOnBrake", std::nearbyint(params.getInt("PauseAOLOnBrake") * speedConversion));
    params.putIntNonBlocking("PauseLateralOnSignal", std::nearbyint(params.getInt("PauseLateralOnSignal") * speedConversion));
    params.putIntNonBlocking("PauseLateralSpeed", std::nearbyint(params.getInt("PauseLateralSpeed") * speedConversion));
    params.putIntNonBlocking("SetSpeedOffset", std::nearbyint(params.getInt("SetSpeedOffset") * speedConversion));
    params.putIntNonBlocking("StoppingDistance", std::nearbyint(params.getInt("StoppingDistance") * distanceConversion));
  }

  FrogPilotParamValueControl *customCruiseToggle = static_cast<FrogPilotParamValueControl*>(toggles["CustomCruise"]);
  FrogPilotParamValueControl *customCruiseLongToggle = static_cast<FrogPilotParamValueControl*>(toggles["CustomCruiseLong"]);
  FrogPilotParamValueControl *laneWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["LaneDetectionWidth"]);
  FrogPilotParamValueControl *minimumLaneChangeSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["MinimumLaneChangeSpeed"]);
  FrogPilotParamValueControl *offset1Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset1"]);
  FrogPilotParamValueControl *offset2Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset2"]);
  FrogPilotParamValueControl *offset3Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset3"]);
  FrogPilotParamValueControl *offset4Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset4"]);
  FrogPilotParamValueControl *pauseAOLOnBrakeToggle = static_cast<FrogPilotParamValueControl*>(toggles["PauseAOLOnBrake"]);
  FrogPilotParamValueControl *pauseLateralToggle = static_cast<FrogPilotParamValueControl*>(toggles["PauseLateralSpeed"]);
  FrogPilotParamValueControl *setSpeedOffsetToggle = static_cast<FrogPilotParamValueControl*>(toggles["SetSpeedOffset"]);
  FrogPilotParamValueControl *stoppingDistanceToggle = static_cast<FrogPilotParamValueControl*>(toggles["StoppingDistance"]);

  if (isMetric) {
    offset1Toggle->setTitle(tr("Speed Limit Offset (0-34 kph)"));
    offset2Toggle->setTitle(tr("Speed Limit Offset (35-54 kph)"));
    offset3Toggle->setTitle(tr("Speed Limit Offset (55-64 kph)"));
    offset4Toggle->setTitle(tr("Speed Limit Offset (65-99 kph)"));

    offset1Toggle->setDescription(tr("Set speed limit offset for limits between 0-34 kph."));
    offset2Toggle->setDescription(tr("Set speed limit offset for limits between 35-54 kph."));
    offset3Toggle->setDescription(tr("Set speed limit offset for limits between 55-64 kph."));
    offset4Toggle->setDescription(tr("Set speed limit offset for limits between 65-99 kph."));

    customCruiseToggle->updateControl(1, 150, tr(" kph"));
    customCruiseLongToggle->updateControl(1, 150, tr(" kph"));
    minimumLaneChangeSpeedToggle->updateControl(0, 150, tr(" kph"));
    offset1Toggle->updateControl(-99, 99, tr(" kph"));
    offset2Toggle->updateControl(-99, 99, tr(" kph"));
    offset3Toggle->updateControl(-99, 99, tr(" kph"));
    offset4Toggle->updateControl(-99, 99, tr(" kph"));
    pauseAOLOnBrakeToggle->updateControl(0, 99, tr(" kph"));
    pauseLateralToggle->updateControl(0, 99, tr(" kph"));
    setSpeedOffsetToggle->updateControl(0, 150, tr(" kph"));

    laneWidthToggle->updateControl(0, 30, tr(" meters"), 10);
    stoppingDistanceToggle->updateControl(0, 5, tr(" meters"));
  } else {
    offset1Toggle->setTitle(tr("Speed Limit Offset (0-34 mph)"));
    offset2Toggle->setTitle(tr("Speed Limit Offset (35-54 mph)"));
    offset3Toggle->setTitle(tr("Speed Limit Offset (55-64 mph)"));
    offset4Toggle->setTitle(tr("Speed Limit Offset (65-99 mph)"));

    offset1Toggle->setDescription(tr("Set speed limit offset for limits between 0-34 mph."));
    offset2Toggle->setDescription(tr("Set speed limit offset for limits between 35-54 mph."));
    offset3Toggle->setDescription(tr("Set speed limit offset for limits between 55-64 mph."));
    offset4Toggle->setDescription(tr("Set speed limit offset for limits between 65-99 mph."));

    customCruiseToggle->updateControl(1, 99, tr(" mph"));
    customCruiseLongToggle->updateControl(1, 99, tr(" mph"));
    minimumLaneChangeSpeedToggle->updateControl(0, 99, tr(" mph"));
    offset1Toggle->updateControl(-99, 99, tr(" mph"));
    offset2Toggle->updateControl(-99, 99, tr(" mph"));
    offset3Toggle->updateControl(-99, 99, tr(" mph"));
    offset4Toggle->updateControl(-99, 99, tr(" mph"));
    pauseAOLOnBrakeToggle->updateControl(0, 99, tr(" mph"));
    pauseLateralToggle->updateControl(0, 99, tr(" mph"));
    setSpeedOffsetToggle->updateControl(0, 99, tr(" mph"));

    laneWidthToggle->updateControl(0, 100, tr(" feet"), 10);
    stoppingDistanceToggle->updateControl(0, 10, tr(" feet"));
  }

  customCruiseToggle->refresh();
  customCruiseLongToggle->refresh();
  laneWidthToggle->refresh();
  minimumLaneChangeSpeedToggle->refresh();
  offset1Toggle->refresh();
  offset2Toggle->refresh();
  offset3Toggle->refresh();
  offset4Toggle->refresh();
  pauseAOLOnBrakeToggle->refresh();
  pauseLateralToggle->refresh();
  setSpeedOffsetToggle->refresh();
  stoppingDistanceToggle->refresh();
}

void FrogPilotControlsPanel::hideToggles() {
  customPersonalitiesOpen = false;
  slcOpen = false;

  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  deleteModelBtn->setVisible(false);
  downloadModelBtn->setVisible(false);
  personalitiesInfoBtn->setVisible(false);
  selectModelBtn->setVisible(false);

  std::set<QString> longitudinalKeys = {"ConditionalExperimental", "DrivingPersonalities", "ExperimentalModeActivation",
                                        "LongitudinalTune", "MTSCEnabled", "SpeedLimitController", "VisionTurnControl"};

  for (auto &[key, toggle] : toggles) {
    toggle->setVisible(false);

    if (!hasOpenpilotLongitudinal && longitudinalKeys.find(key.c_str()) != longitudinalKeys.end()) {
      continue;
    }

    bool subToggles = aggressivePersonalityKeys.find(key.c_str()) != aggressivePersonalityKeys.end() ||
                      aolKeys.find(key.c_str()) != aolKeys.end() ||
                      conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end() ||
                      customdrivingPersonalityKeys.find(key.c_str()) != customdrivingPersonalityKeys.end() ||
                      relaxedPersonalityKeys.find(key.c_str()) != relaxedPersonalityKeys.end() ||
                      deviceManagementKeys.find(key.c_str()) != deviceManagementKeys.end() ||
                      drivingPersonalityKeys.find(key.c_str()) != drivingPersonalityKeys.end() ||
                      experimentalModeActivationKeys.find(key.c_str()) != experimentalModeActivationKeys.end() ||
                      laneChangeKeys.find(key.c_str()) != laneChangeKeys.end() ||
                      lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end() ||
                      longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end() ||
                      mtscKeys.find(key.c_str()) != mtscKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end() ||
                      relaxedPersonalityKeys.find(key.c_str()) != relaxedPersonalityKeys.end() ||
                      speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end() ||
                      speedLimitControllerControlsKeys.find(key.c_str()) != speedLimitControllerControlsKeys.end() ||
                      speedLimitControllerQOLKeys.find(key.c_str()) != speedLimitControllerQOLKeys.end() ||
                      speedLimitControllerVisualsKeys.find(key.c_str()) != speedLimitControllerVisualsKeys.end() ||
                      standardPersonalityKeys.find(key.c_str()) != standardPersonalityKeys.end() ||
                      relaxedPersonalityKeys.find(key.c_str()) != relaxedPersonalityKeys.end() ||
                      trafficPersonalityKeys.find(key.c_str()) != trafficPersonalityKeys.end() ||
                      tuningKeys.find(key.c_str()) != tuningKeys.end() ||
                      visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end();
    toggle->setVisible(!subToggles);
  }

  update();
}

void FrogPilotControlsPanel::hideSubToggles() {
  if (customPersonalitiesOpen) {
    for (auto &[key, toggle] : toggles) {
      bool isVisible = drivingPersonalityKeys.find(key.c_str()) != drivingPersonalityKeys.end();
      toggle->setVisible(isVisible);
    }
  } else if (slcOpen) {
    for (auto &[key, toggle] : toggles) {
      bool isVisible = speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end();
      toggle->setVisible(isVisible);
    }
  }

  update();
}

void FrogPilotControlsPanel::hideSubSubToggles() {
  personalitiesInfoBtn->setVisible(false);

  for (auto &[key, toggle] : toggles) {
    bool isVisible = customdrivingPersonalityKeys.find(key.c_str()) != customdrivingPersonalityKeys.end();
    toggle->setVisible(isVisible);
  }

  update();
}
