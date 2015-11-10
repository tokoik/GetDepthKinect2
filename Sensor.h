#pragma once

//
// 深度センサ関連の処理
//

// Kinect 関連
#include <Windows.h>
#include <Kinect.h>

// ウィンドウ関連の処理
#include "Window.h"

// NUI_IMAGE_RESOLUTION の設定
#define EXPAND_RESOLUTION(width, height) NUI_IMAGE_RESOLUTION_##width##x##height
#define RESOLUTION(width, height) EXPAND_RESOLUTION(width, height)

class Sensor
{
  // センサの識別子
  static IKinectSensor *sensor;

  // 座標のマッピング
  ICoordinateMapper *coordinateMapper;

  // カラーデータ
  IColorFrameSource *colorSource;
  IColorFrameReader *colorReader;
  IFrameDescription *colorDescription;

  // カラーデータのサイズ
  int color_w, color_h, color_size;

  // カラーデータ変換用の一時バッファ
  BYTE *color_buffer;

  // カラーデータを格納するテクスチャ
  GLuint colorTexture;

  // デプスデータ
  IDepthFrameSource *depthSource;
  IDepthFrameReader *depthReader;
  IFrameDescription *depthDescription;

  // デプスデータのサイズ
  int depth_w, depth_h, depth_count;

  // デプスデータを格納するテクスチャ
  GLuint depthTexture;

  // 描画に使う頂点配列オブジェクト
  GLuint vao;

  // 描画に使う頂点バッファオブジェクト
  GLuint vbo[3];

  // 描画する三角形の頂点の総数
  GLsizei vertexCount;

  // コピーコンストラクタ (コピー禁止)
  Sensor(const Sensor &w);

  // 代入 (代入禁止)
  Sensor &operator=(const Sensor &w);

public:

  // コンストラクタ
  Sensor();

  // デストラクタ
  virtual ~Sensor();

  // カラーデータを取得する
  bool getColor() const;

  // デプスデータを取得する
  bool getDepth() const;

  // 使用しているセンサーの数を調べる
  int getActivated() const
  {
    return sensor != NULL ? 1 : 0;
  }

  // 描画
  void draw() const
  {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, NULL);
  }
};
