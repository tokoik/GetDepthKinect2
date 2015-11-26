#pragma once

//
// 深度センサの基底クラス
//

class DepthCamera
{
protected:

  // デプスカメラのサイズ
  int depthWidth, depthHeight;

  // カラーカメラのサイズ
  int colorWidth, colorHeight;

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
  virtual ~DepthCamera()
  {
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
};
