#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include <QObject>
#include <QTimer>
#include <QColor>

#include "nanovg.h"

#include "cereal/messaging/messaging.h"
#include "cereal/visionipc/visionipc.h"
#include "cereal/visionipc/visionipc_client.h"
#include "common/transformations/orientation.hpp"
#include "selfdrive/camerad/cameras/camera_common.h"
#include "selfdrive/common/glutil.h"
#include "selfdrive/common/mat.h"
#include "selfdrive/common/modeldata.h"
#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/common/visionimg.h"

#define COLOR_BLACK nvgRGBA(0, 0, 0, 255)
#define COLOR_BLACK_ALPHA(x) nvgRGBA(0, 0, 0, x)
#define COLOR_WHITE nvgRGBA(255, 255, 255, 255)
#define COLOR_WHITE_ALPHA(x) nvgRGBA(255, 255, 255, x)
#define COLOR_RED nvgRGBA(255, 0, 0, 255)
#define COLOR_RED_ALPHA(x) nvgRGBA(255, 0, 0, x)
#define COLOR_YELLOW nvgRGBA(255, 255, 0, 255)
#define COLOR_YELLOW_ALPHA(x) nvgRGBA(255, 255, 0, x)
#define COLOR_ENGAGED nvgRGBA(23, 134, 68, 255)
#define COLOR_ENGAGED_ALPHA(x) nvgRGBA(23, 134, 68, x)
#define COLOR_WARNING nvgRGBA(218, 111, 37, 255)
#define COLOR_WARNING_ALPHA(x) nvgRGBA(218, 111, 37, x)
#define COLOR_ENGAGEABLE nvgRGBA(23, 51, 73, 255)
#define COLOR_ENGAGEABLE_ALPHA(x) nvgRGBA(23, 51, 73, x)

// TODO: this is also hardcoded in common/transformations/camera.py
// TODO: choose based on frame input size
const float y_offset = Hardware::TICI() ? 150.0 : 0.0;
const float ZOOM = Hardware::TICI() ? 2912.8 : 2138.5;

typedef struct Rect {
  int x, y, w, h;
  int centerX() const { return x + w / 2; }
  int centerY() const { return y + h / 2; }
  int right() const { return x + w; }
  int bottom() const { return y + h; }
  bool ptInRect(int px, int py) const {
    return px >= x && px < (x + w) && py >= y && py < (y + h);
  }
} Rect;

const int sbr_w = 300;
const int bdr_s = 10;
const int header_h = 420;
const int footer_h = 280;

const int UI_FREQ = 20;   // Hz

typedef enum UIStatus {
  STATUS_DISENGAGED,
  STATUS_ENGAGED,
  STATUS_WARNING,
  STATUS_ALERT,
} UIStatus;

const QColor bg_colors [] = {
  [STATUS_DISENGAGED] =  QColor(0x17, 0x33, 0x49, 0xc8),
  [STATUS_ENGAGED] = QColor(0x17, 0x86, 0x44, 0x01),
  [STATUS_WARNING] = QColor(0xDA, 0x6F, 0x25, 0x01),
  [STATUS_ALERT] = QColor(0xC9, 0x22, 0x31, 0xf1),
};

typedef struct {
  float x, y;
} vertex_data;

typedef struct {
  vertex_data v[TRAJECTORY_SIZE * 2];
  int cnt;
} line_vertices_data;

typedef struct UIScene {

  mat3 view_from_calib;
  bool world_objects_visible;

  // ui add
  bool leftBlinker, rightBlinker;
  bool leftblindspot, rightblindspot;
  int blinker_blinkingrate;
  bool batteryCharging;
  int batteryPercent;
  char batteryStatus[64];
  float cpuTempAvg;
  float tpmsFl, tpmsFr, tpmsRl, tpmsRr;
  int lateralControlSelect;
  float output_scale;
  bool ui_tpms;

  cereal::PandaState::PandaType pandaType;

  cereal::DeviceState::Reader deviceState;
  cereal::RadarState::LeadData::Reader lead_data[2];
  cereal::CarState::Reader car_state;
  cereal::ControlsState::Reader controls_state;

  // gps
  int satelliteCount;
  float gpsAccuracy;

  // modelV2
  float lane_line_probs[4];
  float road_edge_stds[2];
  line_vertices_data track_vertices;
  line_vertices_data lane_line_vertices[4];
  line_vertices_data road_edge_vertices[2];

  bool dm_active, engageable;

  // lead
  vertex_data lead_vertices[2];

  float light_sensor, accel_sensor, gyro_sensor;
  bool started, ignition, is_metric, longitudinal_control, end_to_end;
  uint64_t started_frame;

  // add
  cereal::CarControl::Reader car_control;
} UIScene;

typedef struct UIState {
  VisionIpcClient * vipc_client;
  VisionIpcClient * vipc_client_rear;
  VisionIpcClient * vipc_client_wide;
  VisionBuf * last_frame;

  // framebuffer
  int fb_w, fb_h;

  // NVG
  NVGcontext *vg;

  // images
  std::map<std::string, int> images;

  std::unique_ptr<SubMaster> sm;

  UIStatus status;
  UIScene scene;

  // graphics
  std::unique_ptr<GLShader> gl_shader;
  std::unique_ptr<EGLImageTexture> texture[UI_BUF_COUNT];

  GLuint frame_vao, frame_vbo, frame_ibo;
  mat4 rear_frame_mat;

  bool awake;

  Rect video_rect, viz_rect;
  float car_space_transform[6];
  bool wide_camera;
} UIState;


class QUIState : public QObject {
  Q_OBJECT

public:
  QUIState(QObject* parent = 0);

  // TODO: get rid of this, only use signal
  inline static UIState ui_state = {0};

signals:
  void uiUpdate(const UIState &s);
  void offroadTransition(bool offroad);

private slots:
  void update();

private:
  QTimer *timer;
  bool started_prev = true;
};


// device management class

class Device : public QObject {
  Q_OBJECT

public:
  Device(QObject *parent = 0);

private:
  // auto brightness
  const float accel_samples = 5*UI_FREQ;

  bool awake;
  int awake_timeout = 0;
  float accel_prev = 0;
  float gyro_prev = 0;
  float last_brightness = 0;
  FirstOrderFilter brightness_filter;

  QTimer *timer;

  void updateBrightness(const UIState &s);
  void updateWakefulness(const UIState &s);

signals:
  void displayPowerChanged(bool on);

public slots:
  void setAwake(bool on, bool reset);
  void update(const UIState &s);
};
