#pragma once

#include <QPushButton>

#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"

// SSH enable toggle
class SshToggle : public ToggleControl {
  Q_OBJECT

public:
  SshToggle() : ToggleControl("SSH 사용", "", "../assets/offroad/icon_ssh.png", Hardware::get_ssh_enabled()) {
    QObject::connect(this, &SshToggle::toggleFlipped, [=](bool state) {
      Hardware::set_ssh_enabled(state);
    });
  }
};

// SSH key management widget
class SshControl : public AbstractControl {
  Q_OBJECT

public:
  SshControl();

private:
  Params params;

  QPushButton btn;
  QLabel username_label;

  void refresh();
  void getUserKeys(const QString &username);
};

// prebuilt
class PrebuiltToggle : public ToggleControl {
  Q_OBJECT

public:
  PrebuiltToggle() : ToggleControl("Prebuilt 파일 생성", "Prebuilt 파일을 생성하며 부팅속도를 향상시킵니다.", "../assets/offroad/icon_prebuilt.png", Params().getBool("PutPrebuilt")) {
    QObject::connect(this, &PrebuiltToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().put("PutPrebuilt", &value, 1);
    });
  }
};

// Shutdownd
class ShutdowndToggle : public ToggleControl {
  Q_OBJECT

public:
  ShutdowndToggle() : ToggleControl("Shutdownd 사용", "Shutdownd (시동 off 5분) 자동종료를 사용합니다.", "../assets/offroad/icon_shutdownd.png", Params().getBool("Shutdownd")) {
    QObject::connect(this, &ShutdowndToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().put("Shutdownd", &value, 1);
    });
  }
};

// DisableLogger
class DisableLoggerToggle : public ToggleControl {
  Q_OBJECT

public:
  DisableLoggerToggle() : ToggleControl("Logger Disable", "Logger 프로세스를 종료하여 시스템 부하를 줄입니다.", "../assets/offroad/icon_logger.png", Params().getBool("DisableLogger")) {
    QObject::connect(this, &DisableLoggerToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().put("DisableLogger", &value, 1);
    });
  }
};


// MfcSelect
class MfcSelect : public AbstractControl {
  Q_OBJECT

public:
  MfcSelect();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

// LateralControlSelect
class LateralControlSelect : public AbstractControl {
  Q_OBJECT

public:
  LateralControlSelect();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};