#pragma once

//
// 深度センサ関連の処理
//

// Kinect 関連
#include <Windows.h>
#include <Kinect.h>

// ウィンドウ関連の処理
#include "Window.h"

// 深度センサ関連の基底クラス
#include "DepthCamera.h"

class KinectV2 : public DepthCamera
{
  // センサの識別子
  static IKinectSensor *sensor;

  // 座標のマッピング
  ICoordinateMapper *coordinateMapper;

  // デプスデータ
  IDepthFrameSource *depthSource;
  IDepthFrameReader *depthReader;
  IFrameDescription *depthDescription;

  // デプスデータの画素数
  int depthCount;

  // デプスデータを格納するテクスチャ
  GLuint depthTexture;

  // デプスデータから変換したポイントのカメラ座標を格納するテクスチャ
  GLuint pointTexture;

  // デプスデータからカメラ座標を求めるときに用いる一時メモリ
  GLfloat (*position)[3];

  // カラーデータ
  IColorFrameSource *colorSource;
  IColorFrameReader *colorReader;
  IFrameDescription *colorDescription;

  // カラーデータの画素数
  int colorCount;

  // カラーデータを格納するテクスチャ
  GLuint colorTexture;

  // カラーデータの変換に用いる一時メモリ
  GLubyte *color;

  // デプスデータの画素におけるカラーデータのテクスチャ座標値を格納するバッファオブジェクト
  GLuint coordBuffer;

  // コピーコンストラクタ (コピー禁止)
  KinectV2(const KinectV2 &w);

  // 代入 (代入禁止)
  KinectV2 &operator=(const KinectV2 &w);

public:

  // コンストラクタ
  KinectV2();

  // デストラクタ
  virtual ~KinectV2();

  // デプスデータを取得する
  GLuint getDepth() const;

  // カメラ座標を取得する
  GLuint getPoint() const;

  // カラーデータを取得する
  GLuint getColor() const;

  // デプスデータの解像度を取得する
  void getDepthResolution(int *width, int *height) const
  {
    *width = depthWidth;
    *height = depthHeight;
  }

  // カラーデータの解像度を取得する
  void getColorResolution(int *width, int *height) const
  {
    *width = colorWidth;
    *height = colorHeight;
  }

  // カラーデータのテクスチャ座標値を格納するバッファオブジェクトを得る
  GLuint getCoordBuffer() const
  {
    return coordBuffer;
  }

  // 使用しているセンサーの数を調べる
  int getActivated() const
  {
    return sensor != NULL ? 1 : 0;
  }
};
