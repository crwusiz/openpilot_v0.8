#pragma once

#include <QTimer>
#include <QPushButton>
#include <QNetworkAccessManager>

#include "widgets/controls.hpp"
#include "selfdrive/hardware/hw.h"

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

// prebuilt
class PrebuiltToggle : public ToggleControl {
  Q_OBJECT

public:
  PrebuiltToggle() : ToggleControl("Prebuilt 파일 생성", "Prebuilt 파일을 생성하며 부팅속도를 향상시킵니다.", "../assets/offroad/icon_prebuilt.png", Params().read_db_bool("PutPrebuilt")) {
    QObject::connect(this, &PrebuiltToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("PutPrebuilt", &value, 1);
    });
  }
};

// Shutdownd
class ShutdowndToggle : public ToggleControl {
  Q_OBJECT

public:
  ShutdowndToggle() : ToggleControl("Shutdownd Disable", "Shutdownd (시동 종료후 5분후 자동종료)를 사용하지않습니다. 배터리 없는 기종은 활성화 하세요.", "../assets/offroad/icon_shutdownd.png", Params().read_db_bool("Shutdownd")) {
    QObject::connect(this, &ShutdowndToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("Shutdownd", &value, 1);
    });
  }
};

// DisableLogger
class DisableLoggerToggle : public ToggleControl {
  Q_OBJECT

public:
  DisableLoggerToggle() : ToggleControl("Logger Disable", "Logger 프로세스를 종료하여 시스템 부하를 줄입니다.", "../assets/offroad/icon_logger.png", Params().read_db_bool("DisableLogger")) {
    QObject::connect(this, &DisableLoggerToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("DisableLogger", &value, 1);
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

// SSH key management widget
class SshControl : public AbstractControl {
  Q_OBJECT

public:
  SshControl();

private:
  QPushButton btn;
  QString username;
  QLabel username_label;

  // networking
  QTimer* networkTimer;
  QNetworkReply* reply;
  QNetworkAccessManager* manager;

  void refresh();
  void getUserKeys(QString username);

signals:
  void failedResponse(QString errorString);

private slots:
  void timeout();
  void parseResponse();
};

/* =====================================================================================================================
// LDWS MFC
class LdwsToggle : public ToggleControl {
  Q_OBJECT

public:
  LdwsToggle() : ToggleControl("LDWS MFC", "", "../assets/offroad/icon_ldwsmfc.png", Params().read_db_bool("LdwsMfc")) {
    QObject::connect(this, &LdwsToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LdwsMfc", &value, 1);
    });
  }
};

// LFA MFC
class LfaToggle : public ToggleControl {
  Q_OBJECT

public:
  LfaToggle() : ToggleControl("LFA MFC", "", "../assets/offroad/icon_lfamfc.png", Params().read_db_bool("LfaMfc")) {
    QObject::connect(this, &LfaToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LfaMfc", &value, 1);
    });
  }
};

// MadMode
class MadModeToggle : public ToggleControl {
  Q_OBJECT

public:
  MadModeToggle() : ToggleControl("MAD 모드 사용", "", "../assets/offroad/icon_warning.png", Params().read_db_bool("MadModeEnabled")) {
    QObject::connect(this, &MadModeToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("MadModeEnabled", &value, 1);
    });
  }
};

// LongControl
class LongControlToggle : public ToggleControl {
  Q_OBJECT

public:
  LongControlToggle() : ToggleControl("LongControl 사용", "", "../assets/offroad/icon_long.png", Params().read_db_bool("LongControlEnabled")) {
    QObject::connect(this, &LongControlToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().write_db_value("LongControlEnabled", &value, 1);
    });
  }
};
*/
