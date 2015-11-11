#include "KinectV1.h"

//
// 深度センサ関連の処理
//

// 標準ライブラリ
#include <cassert>

// Kinect 関連
#pragma comment(lib, "Kinect10.lib")

// コンストラクタ
Sensor::Sensor()
  : texcoord(new GLfloat [DEPTH_W * DEPTH_H][2])
  , nextColorFrameEvent(CreateEvent(NULL, TRUE, FALSE, NULL))
  , nextDepthFrameEvent(CreateEvent(NULL, TRUE, FALSE, NULL))
{
  // 最初のインスタンスを生成するときだけ
  if (activated == 0)
  {
    // 接続されているセンサの数を調べる
    NuiGetSensorCount(&connected);
  }

  // センサが接続されており使用台数が接続台数に達していなければ
  if (activated < connected)
  {
    // センサの使用を開始する
    NuiCreateSensorByIndex(activated, &sensor);

    // センサが使用可能であれば
    if (sensor->NuiStatus() == S_OK)
    {
      // センサを初期化する (カラーとデプスを取得する)
      assert(sensor->NuiInitialize(
        NUI_INITIALIZE_FLAG_USES_COLOR |
        NUI_INITIALIZE_FLAG_USES_DEPTH |
        0) == S_OK);

      // センサの仰角を初期化する
      assert(sensor->NuiCameraElevationSetAngle(0L) == S_OK);

      // デプスストリームの取得設定
      assert(sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, RESOLUTION(DEPTH_W, DEPTH_H),
        0, 2, nextDepthFrameEvent, &depthStream) == S_OK);

      // デプスを格納するテクスチャを準備する
      glGenTextures(1, &depthTexture);
      glBindTexture(GL_TEXTURE_2D, depthTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, DEPTH_W, DEPTH_H, 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      // カラーストリームの取得設定
      assert(sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, RESOLUTION(COLOR_W, COLOR_H),
        0, 2, nextColorFrameEvent, &colorStream) == S_OK);

      // カラーを格納するテクスチャを準備する
      glGenTextures(1, &colorTexture);
      glBindTexture(GL_TEXTURE_2D, colorTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, COLOR_W, COLOR_H, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      // 使用しているセンサの数を数える
      ++activated;
    }
  }
}

// デストラクタ
Sensor::~Sensor()
{
  // カラーのテクスチャ座標用のメモリを開放する
  delete[] texcoord;

  // センサが有効になっていたら
  if (activated > 0)
  {
    // センサをシャットダウンする
    sensor->NuiShutdown();

    // テクスチャを削除する
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(1, &depthTexture);

    // 使用しているセンサの数を減らす
    --activated;
  }
}

// データを取得する
bool Sensor::getImage(HANDLE event, HANDLE stream,
  GLuint texture, GLsizei width, GLsizei height, GLenum format, GLenum type) const
{
  // カラーのテクスチャを指定する
  glBindTexture(GL_TEXTURE_2D, texture);

  // 次のフレームデータが到着していれば
  if (WaitForSingleObject(event, 0) != WAIT_TIMEOUT)
  {
    // 取得したフレームデータの格納場所
    NUI_IMAGE_FRAME frame;

    // フレームデータの格納場所を frame に取得する
    if (sensor->NuiImageStreamGetNextFrame(stream, 0, &frame) == S_OK)
    {
      // これから処理完了までデータが変更されないようにロックする
      NUI_LOCKED_RECT rect;
      frame.pFrameTexture->LockRect(0, &rect, NULL, 0);

      // ロックに成功したら
      if (rect.Pitch)
      {
        // pBits に入っているデータをテクスチャに転送する
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, rect.pBits);

        // 取得したのがデプスデータであれば
        if (texture == depthTexture)
        {
          // カラーデータのテクスチャ座標を求めて texcoord に保存する
          for (int i = 0; i < DEPTH_W * DEPTH_H; ++i)
          {
            // デプスの画素位置からカラーの画素位置を求める
            LONG x, y;
            sensor->NuiImageGetColorPixelCoordinatesFromDepthPixel(RESOLUTION(COLOR_W, COLOR_H),
              NULL, i % DEPTH_W, i / DEPTH_W, reinterpret_cast<USHORT *>(rect.pBits)[i], &x, &y);

            // カラーデータのテクスチャ座標に変換する
            texcoord[i][0] = (GLfloat(x) + 0.5f) / GLfloat(COLOR_W);
            texcoord[i][1] = (GLfloat(y) + 0.5f) / GLfloat(COLOR_H);
          }
        }

        // ロックしたデータを開放する
        assert(sensor->NuiImageStreamReleaseFrame(stream, &frame) == S_OK);

        return true;
      }
    }
  }

  return false;
}

// 接続しているセンサの数
int Sensor::connected(0);

// 使用しているセンサの数
int Sensor::activated(0);
