#include <QNetworkReply>
#include <QHBoxLayout>
#include "widgets/input.hpp"
#include "widgets/ssh_keys.hpp"
#include "common/params.h"


SshControl::SshControl() : AbstractControl("SSH키 변경", "Github 사용자 id에 등록된 ssh-rsa키로 변경됩니다.", "../assets/offroad/icon_ssh.png") {

  // setup widget
  hlayout->addStretch(1);

  username_label.setAlignment(Qt::AlignVCenter);
  username_label.setStyleSheet("color: #aaaaaa");
  hlayout->addWidget(&username_label);

  btn.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btn.setFixedSize(250, 100);
  hlayout->addWidget(&btn);

  QObject::connect(&btn, &QPushButton::released, [=]() {
    if (btn.text() == "추가") {
      username = InputDialog::getText("GitHub ID");
      if (username.length() > 0) {
        btn.setText("로딩중");
        btn.setEnabled(false);
        getUserKeys(username);
      }
    } else {
      Params().delete_db_value("GithubUsername");
      Params().delete_db_value("GithubSshKeys");
      refresh();
    }
  });

  // setup networking
  manager = new QNetworkAccessManager(this);
  networkTimer = new QTimer(this);
  networkTimer->setSingleShot(true);
  networkTimer->setInterval(5000);
  connect(networkTimer, SIGNAL(timeout()), this, SLOT(timeout()));

  refresh();
}

void SshControl::refresh() {
  QString param = QString::fromStdString(Params().get("GithubSshKeys"));
  if (param.length()) {
    username_label.setText(QString::fromStdString(Params().get("GithubUsername")));
    btn.setText("제거");
  } else {
    username_label.setText("");
    btn.setText("추가");
  }
  btn.setEnabled(true);
}

void SshControl::getUserKeys(QString username){
  QString url = "https://github.com/" + username + ".keys";

  QNetworkRequest request;
  request.setUrl(QUrl(url));
#ifdef QCOM
  QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
  ssl.setCaCertificates(QSslCertificate::fromPath("/usr/etc/tls/cert.pem",
                        QSsl::Pem, QRegExp::Wildcard));
  request.setSslConfiguration(ssl);
#endif

  reply = manager->get(request);
  connect(reply, SIGNAL(finished()), this, SLOT(parseResponse()));
  networkTimer->start();
}

void SshControl::timeout(){
  reply->abort();
}

void SshControl::parseResponse(){
  QString err = "";
  if (reply->error() != QNetworkReply::OperationCanceledError) {
    networkTimer->stop();
    QString response = reply->readAll();
    if (reply->error() == QNetworkReply::NoError && response.length()) {
      Params().write_db_value("GithubUsername", username.toStdString());
      Params().write_db_value("GithubSshKeys", response.toStdString());
    } else if(reply->error() == QNetworkReply::NoError){
      err = username + " 에 등록된 ssh-rsa키가 없습니다.";
    } else {
      err = username + " 이 등록된 Github id가 아닙니다.";
    }
  } else {
    err = "요청시간이 초과되었습니다.";
  }

  if (err.length()) {
    ConfirmationDialog::alert(err);
  }

  refresh();
  reply->deleteLater();
  reply = nullptr;
}

LateralControl::LateralControl() : AbstractControl("조향로직", "조향로직을 설정합니다. (PID/INDI/LQR)", "../assets/offroad/icon_logic.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LateralControlMethod"));
    int latcontrol = str.toInt();
    latcontrol = latcontrol - 1;
    if (latcontrol <= 0 ) {
      latcontrol = 0;
    } else {
    }
    QString latcontrols = QString::number(latcontrol);
    Params().write_db_value("LateralControlMethod", latcontrols.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("LateralControlMethod"));
    int latcontrol = str.toInt();
    latcontrol = latcontrol + 1;
    if (latcontrol >= 2 ) {
      latcontrol = 2;
    } else {
    }
    QString latcontrols = QString::number(latcontrol);
    Params().write_db_value("LateralControlMethod", latcontrols.toStdString());
    refresh();
  });
  refresh();
}

void LateralControl::refresh() {
  QString latcontrol = QString::fromStdString(Params().get("LateralControlMethod"));
  if (latcontrol == "0") {
    label.setText(QString::fromStdString("PID"));
  } else if (latcontrol == "1") {
    label.setText(QString::fromStdString("INDI"));
  } else if (latcontrol == "2") {
    label.setText(QString::fromStdString("LQR"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}

MfcSelect::MfcSelect() : AbstractControl("MFC 선택", "MFC를 선택합니다. (LKAS/LDWS/LFA)", "../assets/offroad/icon_mfc.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MfcSelect"));
    int mfc = str.toInt();
    mfc = mfc - 1;
    if (mfc <= 0 ) {
      mfc = 0;
    } else {
    }
    QString mfcs = QString::number(mfc);
    Params().write_db_value("MfcSelect", mfcs.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::released, [=]() {
    auto str = QString::fromStdString(Params().get("MfcSelect"));
    int mfc = str.toInt();
    mfc = mfc + 1;
    if (mfc >= 2 ) {
      mfc = 2;
    } else {
    }
    QString mfcs = QString::number(mfc);
    Params().write_db_value("MfcSelect", mfcs.toStdString());
    refresh();
  });
  refresh();
}

void MfcSelect::refresh() {
  QString mfc = QString::fromStdString(Params().get("MfcSelect"));
  if (mfc == "0") {
    label.setText(QString::fromStdString("LKAS"));
  } else if (mfc == "1") {
    label.setText(QString::fromStdString("LDWS"));
  } else if (mfc == "2") {
    label.setText(QString::fromStdString("LFA"));
  }
  btnminus.setText("◀");
  btnplus.setText("▶");
}
