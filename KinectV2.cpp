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

    // カラーデータの読み込み設定
    assert(sensor->get_ColorFrameSource(&colorSource) == S_OK);
    assert(colorSource->OpenReader(&colorReader) == S_OK);
    assert(colorSource->get_FrameDescription(&colorDescription) == S_OK);

    // カラーデータのサイズを得る
    colorDescription->get_Width(&color_w);
    colorDescription->get_Height(&color_h);

    // カラーデータ変換用の一時バッファを確保する
    color_size = color_w * color_h * 4;
    color_buffer = new BYTE[color_size];

    // カラーを格納するテクスチャを準備する
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, color_w, color_h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータの読み込み設定
    assert(sensor->get_DepthFrameSource(&depthSource) == S_OK);
    assert(depthSource->OpenReader(&depthReader) == S_OK);
    assert(depthSource->get_FrameDescription(&depthDescription) == S_OK);

    // デプスデータのサイズを得る
    depthDescription->get_Width(&depth_w);
    depthDescription->get_Height(&depth_h);
    depth_count = depth_w * depth_h;

    // デプスを格納するテクスチャを準備する
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, depth_w, depth_h, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 描画する三角形の頂点の総数を求める
    vertexCount = ((depth_w - 1) * (depth_h - 1) * 3 * 2);
      
      // 描画に使うメッシュデータの頂点配列オブジェクトを準備する
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 描画に使うメッシュデータの頂点バッファオブジェクトを準備する
    glGenBuffers(sizeof vbo / sizeof vbo[0], vbo);

    // カラーマップをデプスマップの画素単位にサンプリングするためのバッファオブジェクト
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, depth_count * sizeof(ColorSpacePoint), NULL, GL_DYNAMIC_DRAW);

    // インデックスが 0 の varying 変数に割り当てる
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    // デプスマップから求めたカメラ座標系の位置
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, depth_count * sizeof(CameraSpacePoint), NULL, GL_DYNAMIC_DRAW);

    // インデックスが 1 の varying 変数に割り当てる
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    // インデックス用のバッファオブジェクト
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(GLuint), NULL, GL_STATIC_DRAW);

    // インデックスを求めてバッファオブジェクトに転送する
    GLuint *index(static_cast<GLuint *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)));
    for (int j = 0; j < depth_h - 1; ++j)
    {
      for (int i = 0; i < depth_w - 1; ++i)
      {
        index[0] = depth_w * j + i;
        index[1] = index[5] = index[0] + 1;
        index[2] = index[4] = index[0] + depth_w;
        index[3] = index[2] + 1;
        index += 6;
      }
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  }
}

// デストラクタ
KinectV2::~KinectV2()
{
  if (sensor != NULL)
  {
    // データ変換用の一時バッファを開放する
    delete[] color_buffer;

    // 頂点配列オブジェクトを削除する
    glDeleteVertexArrays(1, &vao);

    // 頂点バッファオブジェクトを削除する
    glDeleteBuffers(3, vbo);

    // センサをリリースする
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
    colorFrame->CopyConvertedFrameDataToArray(color_size, color_buffer,
      ColorImageFormat::ColorImageFormat_Bgra);

    // カラーデータをテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, color_w, color_h,
      GL_RGBA, GL_UNSIGNED_BYTE, color_buffer);

    return true;
  }

  return false;
}

// デプスデータを取得する
bool KinectV2::getDepth() const
{
  // デプスのテクスチャを指定する
  glBindTexture(GL_TEXTURE_2D, depthTexture);

  // 次のカラーのフレームデータが到着していれば
  IDepthFrame *depthFrame;
  if (depthReader->AcquireLatestFrame(&depthFrame) == S_OK)
  {
    // デプスデータのサイズと格納場所を得る
    UINT depth_size;
    UINT16 *depth_buffer;
    depthFrame->AccessUnderlyingBuffer(&depth_size, &depth_buffer);

    // デプスデータをテクスチャに転送する
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depth_w, depth_h,
      GL_RED, GL_UNSIGNED_SHORT, depth_buffer);

    // カラーのテクスチャ座標を作成してバッファオブジェクトに転送する
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    ColorSpacePoint *const texCoord(static_cast<ColorSpacePoint *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
    coordinateMapper->MapDepthFrameToColorSpace(depth_count, depth_buffer, depth_count, texCoord);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // カメラ座標系の頂点位置を求めてバッファオブジェクトに転送する
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    CameraSpacePoint *const position(static_cast<CameraSpacePoint *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
    coordinateMapper->MapDepthFrameToCameraSpace(depth_count, depth_buffer, depth_count, position);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    return true;
  }

  return false;
}

// センサの識別子
IKinectSensor *KinectV2::sensor(NULL);
