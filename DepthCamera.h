#pragma once

//
// 深度センサの基底クラス
//

class DepthCamera
{
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
  void makeTexture()
  {
    // デプスデータとカラーデータの画素数を求める
    depthCount = depthWidth * depthHeight;
    colorCount = colorWidth * colorHeight;

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

    // カラーデータを格納するテクスチャを準備する
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colorWidth, colorHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // デプスデータの画素位置のカラーのテクスチャ座標を格納するバッファオブジェクトを準備する
    glGenBuffers(1, &coordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    glBufferData(GL_ARRAY_BUFFER, depthCount * 2 * sizeof (GLfloat), nullptr, GL_DYNAMIC_DRAW);
  }

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
    if (getActivated() > 0)
    {
      // バッファオブジェクトを削除する
      glDeleteBuffers(1, &coordBuffer);

      // テクスチャを削除する
      glDeleteTextures(1, &depthTexture);
      glDeleteTextures(1, &colorTexture);
      glDeleteTextures(1, &pointTexture);
    }
  }

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
  virtual int getActivated() const = 0;
};
