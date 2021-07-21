#include "selfdrive/ui/qt/widgets/input.h"

#include <QPushButton>

#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/hardware/hw.h"

QDialogBase::QDialogBase(QWidget *parent) : QDialog(parent) {
  Q_ASSERT(parent != nullptr);
  parent->installEventFilter(this);
}

bool QDialogBase::eventFilter(QObject *o, QEvent *e) {
  if (o == parent() && e->type() == QEvent::Hide) {
    reject();
  }
  return QDialog::eventFilter(o, e);
}

InputDialog::InputDialog(const QString &prompt_text, QWidget *parent) : QDialogBase(parent) {
  main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(50, 50, 50, 50);
  main_layout->setSpacing(20);

  // build header
  QHBoxLayout *header_layout = new QHBoxLayout();

  label = new QLabel(prompt_text, this);
  label->setStyleSheet(R"(font-size: 75px; font-weight: 500;)");
  header_layout->addWidget(label, 1, Qt::AlignLeft);

  QPushButton* cancel_btn = new QPushButton("취소");
  cancel_btn->setStyleSheet(R"(
    padding: 30px;
    padding-right: 45px;
    padding-left: 45px;
    border-radius: 7px;
    font-size: 45px;
    background-color: #444444;
  )");
  header_layout->addWidget(cancel_btn, 0, Qt::AlignRight);
  QObject::connect(cancel_btn, &QPushButton::released, this, &InputDialog::reject);
  QObject::connect(cancel_btn, &QPushButton::released, this, &InputDialog::cancel);

  main_layout->addLayout(header_layout);

  // text box
  main_layout->addSpacing(20);
  line = new QLineEdit();
  line->setStyleSheet(R"(
    border: none;
    background-color: #444444;
    font-size: 80px;
    font-weight: 500;
    padding: 10px;
  )");
  main_layout->addWidget(line, 1, Qt::AlignTop);

  k = new Keyboard(this);
  QObject::connect(k, &Keyboard::emitButton, this, &InputDialog::handleInput);
  main_layout->addWidget(k, 2, Qt::AlignBottom);

  setStyleSheet(R"(
    * {
      outline: none;
      color: white;
      font-family: Inter;
      background-color: black;
    }
  )");

}

QString InputDialog::getText(const QString &prompt, QWidget *parent, int minLength, const QString &defaultText) {
  InputDialog d = InputDialog(prompt, parent);
  d.line->setText(defaultText);
  d.setMinLength(minLength);
  const int ret = d.exec();
  return ret ? d.text() : QString();
}

QString InputDialog::text() {
  return line->text();
}

int InputDialog::exec() {
  setMainWindow(this);
  return QDialog::exec();
}

void InputDialog::show() {
  setMainWindow(this);
}

void InputDialog::handleInput(const QString &s) {
  if (!QString::compare(s,"⌫")) {
    line->backspace();
  } else if (!QString::compare(s,"⏎")) {
    if (line->text().length() >= minLength) {
      done(QDialog::Accepted);
      emitText(line->text());
    } else {
      setMessage("Need at least "+QString::number(minLength)+" characters!", false);
    }
  } else {
    line->insert(s.left(1));
  }
}

void InputDialog::setMessage(const QString &message, bool clearInputField) {
  label->setText(message);
  if (clearInputField) {
    line->setText("");
  }
}

void InputDialog::setMinLength(int length) {
  minLength = length;
}

ConfirmationDialog::ConfirmationDialog(const QString &prompt_text, const QString &confirm_text, const QString &cancel_text,
                                       QWidget *parent) : QDialogBase(parent) {
  setWindowFlags(Qt::Popup);
  main_layout = new QVBoxLayout(this);
  main_layout->setMargin(25);

  prompt = new QLabel(prompt_text, this);
  prompt->setWordWrap(true);
  prompt->setAlignment(Qt::AlignHCenter);
  prompt->setStyleSheet(R"(font-size: 55px; font-weight: 400;)");
  main_layout->addWidget(prompt, 1, Qt::AlignTop | Qt::AlignHCenter);

  // cancel + confirm buttons
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->setSpacing(20);
  btn_layout->addStretch(1);
  main_layout->addLayout(btn_layout);

  if (cancel_text.length()) {
    QPushButton* cancel_btn = new QPushButton(cancel_text);
    btn_layout->addWidget(cancel_btn, 0, Qt::AlignRight);
    QObject::connect(cancel_btn, &QPushButton::released, this, &ConfirmationDialog::reject);
  }

  if (confirm_text.length()) {
    QPushButton* confirm_btn = new QPushButton(confirm_text);
    btn_layout->addWidget(confirm_btn, 0, Qt::AlignRight);
    QObject::connect(confirm_btn, &QPushButton::released, this, &ConfirmationDialog::accept);
  }

  setFixedSize(900, 350);
  setStyleSheet(R"(
    * {
      color: black;
      background-color: white;
    }
    QPushButton {
      font-size: 40px;
      padding: 30px;
      padding-right: 45px;
      padding-left: 45px;
      border-radius: 7px;
      background-color: #44444400;
    }
  )");
}

bool ConfirmationDialog::alert(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, "확인", "", parent);
  return d.exec();
}

bool ConfirmationDialog::confirm(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, "확인", "취소", parent);
  return d.exec();
}

int ConfirmationDialog::exec() {
   // TODO: make this work without fullscreen
  if (Hardware::TICI()) {
    setMainWindow(this);
  }
  return QDialog::exec();
}
