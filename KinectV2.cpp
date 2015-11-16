#include "KinectV2.h"

//
// 深度センサ関連の処理
//

// 標準ライブラリ
#include <cassert>

// Kinect 関連
#pragma comment(lib, "Kinect20.lib")

// コンストラクタ
KinectV2::KinectV2()
{
  // センサを取得する
  if (GetDefaultKinectSensor(&sensor) == S_OK)
  {
    // センサの使用を開始する
    assert(sensor->Open() == S_OK);

    // 座標のマッピング
    assert(sensor->get_CoordinateMapper(&coordinateMapper) == S_OK);

    // デプスデータの読み込み設定
    assert(sensor->get_DepthFrameSource(&depthSource) == S_OK);
    assert(depthSource->OpenReader(&depthReader) == S_OK);
    assert(depthSource->get_FrameDescription(&depthDescription) == S_OK);

    // デプスデータのサイズを得る
    depthDescription->get_Width(&depthWidth);
    depthDescription->get_Height(&depthHeight);

    // デプスデータを格納するテクスチャを準備する
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, depthWidth, depthHeight, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータ変換用の一時バッファを確保する
    pointCount = depthWidth * depthHeight;
    pointSize = pointCount * 3 * sizeof (GLfloat);
    pointBuffer = new GLfloat[pointCount][3];

    // デプスデータから求めたカメラ座標を格納するテクスチャを準備する
    glGenTextures(1, &pointTexture);
    glBindTexture(GL_TEXTURE_2D, pointTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, depthWidth, depthHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータの画素位置のカラーのテクスチャ座標ほ保存するメモリを確保する
    texcoord = new GLfloat[pointCount][2];

    // カラーデータの読み込み設定
    assert(sensor->get_ColorFrameSource(&colorSource) == S_OK);
    assert(colorSource->OpenReader(&colorReader) == S_OK);
    assert(colorSource->get_FrameDescription(&colorDescription) == S_OK);

    // カラーデータのサイズを得る
    colorDescription->get_Width(&colorWidth);
    colorDescription->get_Height(&colorHeight);

    // カラーデータ変換用の一時バッファを確保する
    colorSize = colorWidth * colorHeight * 4;
    colorBuffer = new BYTE[colorSize];

    // カラーを格納するテクスチャを準備する
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
}

// デストラクタ
KinectV2::~KinectV2()
{
  if (sensor != NULL)
  {
    // データ変換用の一時バッファを開放する
    delete[] pointBuffer;
    delete[] colorBuffer;

    // デプスデータの画素位置のカラーのテクスチャ座標ほ保存するメモリを確保する
    delete[] texcoord;

    // センサを開放する
    colorDescription->Release();
    colorReader->Release();
    colorSource->Release();
    depthDescription->Release();
    depthReader->Release();
    depthSource->Release();
    coordinateMapper->Release();
    sensor->Close();
    sensor->Release();
  }
}

// デプスデータを取得する
bool KinectV2::getDepth() const
{
  // デプスのテクスチャを指定する
  glBindTexture(GL_TEXTURE_2D, depthTexture);

  // 次のデプスのフレームデータが到着していれば
  IDepthFrame *depthFrame;
  if (depthReader->AcquireLatestFrame(&depthFrame) == S_OK)
  {
    // デプスデータのサイズと格納場所を得る
    UINT depthSize;
    UINT16 *depthBuffer;
    depthFrame->AccessUnderlyingBuffer(&depthSize, &depthBuffer);

    // カラーのテクスチャ座標を求めて保存する
    coordinateMapper->MapDepthFrameToColorSpace(pointCount, depthBuffer, pointCount,
      reinterpret_cast<ColorSpacePoint *>(texcoord));

    // カメラ座標をテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depthWidth, depthHeight,
      GL_RED, GL_UNSIGNED_SHORT, depthBuffer);

    // デプスフレームを開放する
    depthFrame->Release();

    return true;
  }

  return false;
}

// カメラ座標を取得する
bool KinectV2::getPoint() const
{
  // カメラ座標のテクスチャを指定する
  glBindTexture(GL_TEXTURE_2D, pointTexture);

  // 次のデプスのフレームデータが到着していれば
  IDepthFrame *depthFrame;
  if (depthReader->AcquireLatestFrame(&depthFrame) == S_OK)
  {
    // デプスデータのサイズと格納場所を得る
    UINT depthSize;
    UINT16 *depthBuffer;
    depthFrame->AccessUnderlyingBuffer(&depthSize, &depthBuffer);

    UINT32 entry;
    PointF *table;
    coordinateMapper->GetDepthFrameToCameraSpaceTable(&entry, &table);
    for (unsigned int i = 0; i < entry; ++i)
    {
      pointBuffer[i][2] = depthBuffer[i] == 0.0 ? -10.0f : -0.001f * float(depthBuffer[i]);
      pointBuffer[i][0] = table[i].X * pointBuffer[i][2];
      pointBuffer[i][1] = -table[i].Y * pointBuffer[i][2];
    }

    // カラーのテクスチャ座標を求めて保存する
    coordinateMapper->MapDepthFrameToColorSpace(pointCount, depthBuffer, pointCount,
      reinterpret_cast<ColorSpacePoint *>(texcoord));

    // デプスフレームを開放する
    depthFrame->Release();

    // カメラ座標をテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depthWidth, depthHeight,
      GL_RGB, GL_FLOAT, pointBuffer);

    return true;
  }

  return false;
}

// カラーデータを取得する
bool KinectV2::getColor() const
{
  // カラーのテクスチャを指定する
  glBindTexture(GL_TEXTURE_2D, colorTexture);

  // 次のカラーのフレームデータが到着していれば
  IColorFrame *colorFrame;
  if (colorReader->AcquireLatestFrame(&colorFrame) == S_OK)
  {
    // カラーデータを取得して RGBA 形式に変換する
    colorFrame->CopyConvertedFrameDataToArray(colorSize, colorBuffer,
      ColorImageFormat::ColorImageFormat_Rgba);

    // カラーフレームを開放する
    colorFrame->Release();

    // カラーデータをテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, colorWidth, colorHeight,
      GL_RGBA, GL_UNSIGNED_BYTE, colorBuffer);

    return true;
  }

  return false;
}

// センサの識別子
IKinectSensor *KinectV2::sensor(NULL);
