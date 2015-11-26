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

    // デプスデータの画素数を求める
    depthCount = depthWidth * depthHeight;

    // デプスデータを格納するテクスチャを準備する
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, depthWidth, depthHeight, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータから求めたカメラ座標を格納するテクスチャを準備する
    glGenTextures(1, &pointTexture);
    glBindTexture(GL_TEXTURE_2D, pointTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, depthWidth, depthHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // カラーデータの読み込み設定
    assert(sensor->get_ColorFrameSource(&colorSource) == S_OK);
    assert(colorSource->OpenReader(&colorReader) == S_OK);
    assert(colorSource->get_FrameDescription(&colorDescription) == S_OK);

    // カラーデータのサイズを得る
    colorDescription->get_Width(&colorWidth);
    colorDescription->get_Height(&colorHeight);

    // カラーデータの画素数を求める
    colorCount = colorWidth * colorHeight;

    // カラーデータを格納するテクスチャを準備する
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colorWidth, colorHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータの画素位置のカラーのテクスチャ座標を格納するバッファオブジェクトを準備する
    glGenBuffers(1, &coordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    glBufferData(GL_ARRAY_BUFFER, depthCount * 2 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    // デプスデータからカメラ座標を求めるときに用いる一時メモリを確保する
    position = new GLfloat[depthCount][3];

    // カラーデータを変換する用いる一時メモリを確保する
    color = new GLubyte[colorCount * 4];
  }
}

// デストラクタ
KinectV2::~KinectV2()
{
  if (sensor != NULL)
  {
    // データ変換用のメモリを削除する
    delete[] position;
    delete[] color;

    // バッファオブジェクトを削除する
    glDeleteBuffers(1, &coordBuffer);

    // テクスチャを削除する
    glDeleteTextures(1, &depthTexture);
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(1, &pointTexture);

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

    // カラーのテクスチャ座標を求めて転送する
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    ColorSpacePoint *const texcoord(static_cast<ColorSpacePoint *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
    coordinateMapper->MapDepthFrameToColorSpace(depthCount, depthBuffer, depthCount, texcoord);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // デプスデータをテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depthWidth, depthHeight, GL_RED, GL_UNSIGNED_SHORT, depthBuffer);

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
      position[i][2] = depthBuffer[i] == 0.0 ? -10.0f : -0.001f * float(depthBuffer[i]);
      position[i][0] = table[i].X * position[i][2];
      position[i][1] = -table[i].Y * position[i][2];
    }

    // カラーのテクスチャ座標を求めて転送する
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    ColorSpacePoint *const texcoord(static_cast<ColorSpacePoint *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
    coordinateMapper->MapDepthFrameToColorSpace(depthCount, depthBuffer, depthCount, texcoord);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // デプスフレームを開放する
    depthFrame->Release();

    // カメラ座標をテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
      depthWidth, depthHeight, GL_RGB, GL_FLOAT, position);

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
    colorFrame->CopyConvertedFrameDataToArray(colorCount * 4,
      static_cast<BYTE *>(color), ColorImageFormat::ColorImageFormat_Rgba);

    // カラーフレームを開放する
    colorFrame->Release();

    // カラーデータをテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
      colorWidth, colorHeight, GL_RGBA, GL_UNSIGNED_BYTE, color);

    return true;
  }

  return false;
}

// センサの識別子
IKinectSensor *KinectV2::sensor(NULL);
