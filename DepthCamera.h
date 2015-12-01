#pragma once

//
// 深度センサ関連の基底クラス
//

// ウィンドウ関連の処理
#include "Window.h"

class DepthCamera
{
  // 有効化されたデプスカメラの台数
  static int activated;

protected:

  // デプスカメラのサイズと画素数
  int depthWidth, depthHeight, depthCount;

  // デプスデータを格納するテクスチャ
  GLuint depthTexture;

  // デプスデータから変換したポイントのカメラ座標を格納するテクスチャ
  GLuint pointTexture;

  // カラーカメラのサイズと画素数
  int colorWidth, colorHeight, colorCount;

  // カラーデータを格納するテクスチャ
  GLuint colorTexture;

  // デプスデータの画素におけるカラーデータのテクスチャ座標値を格納するバッファオブジェクト
  GLuint coordBuffer;

  // depthCount と colorCount を計算してテクスチャとバッファオブジェクトを作成する
  void makeTexture();

public:

  // コンストラクタ
  DepthCamera()
  {
  }
  DepthCamera(int depthWidth, int depthHeight, int colorWidth, int colorHeight)
    : depthWidth(depthWidth)
    , depthHeight(depthHeight)
    , colorWidth(colorWidth)
    , colorHeight(colorHeight)
  {
  }

  // デストラクタ
  virtual ~DepthCamera();

  // デプスデータを取得する
  GLuint getDepth() const
  {
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    return depthTexture;
  }

  // カメラ座標を取得する
  GLuint getPoint() const
  {
    glBindTexture(GL_TEXTURE_2D, pointTexture);
    return pointTexture;
  }

  // カラーデータを取得する
  GLuint getColor() const
  {
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    return colorTexture;
  }

  // デプスカメラのサイズを得る
  void getDepthResolution(int *width, int *height) const
  {
    *width = depthWidth;
    *height = depthHeight;
  }

  // カラーカメラのサイズを得る
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
  int getActivated()
  {
    return activated;
  }
};
