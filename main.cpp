//
// Kinect (v1) のデプスマップ取得
//

// 標準ライブラリ
#include <Windows.h>
#include <memory>

// ウィンドウ関連の処理
#include "Window.h"

// センサ関連の処理
#include "Sensor.h"

//
// メインプログラム
//
int main()
{
  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // GLFW の初期化に失敗した
    MessageBox(NULL, TEXT("GLFW の初期化に失敗しました。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // プログラム終了時には GLFW を終了する
  atexit(glfwTerminate);

  // OpenGL Version 3.2 Core Profile を選択する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // ウィンドウを開く
  Window window(640, 480, "Depth Map Viewer");
  if (!window.get())
  {
    // ウィンドウが作成できなかった
    MessageBox(NULL, TEXT("GLFW のウィンドウが開けませんでした。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // 深度センサを有効にする
  Sensor sensor;
  if (sensor.getActivated() == 0)
  {
    // センサが使えなかった
    MessageBox(NULL, TEXT("深度センサを有効にできませんでした。"), TEXT("すまんのう"), MB_OK);
    return EXIT_FAILURE;
  }

  // 矩形データ
  const std::unique_ptr<const GgTriangles> rect(ggRectangle());

  // シェーダ
  GgSimpleShader simple("simple.vert", "simple.frag");

  // uniform 変数の場所
  const GLint colorLoc(glGetUniformLocation(simple.get(), "color"));
  const GLint depthLoc(glGetUniformLocation(simple.get(), "depth"));

  // 背景色を設定する
  glClearColor(background[0], background[1], background[2], background[3]);

  // 隠面消去処理を有効にする
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // ウィンドウが開いている間くり返し描画する
  while (!window.shouldClose())
  {
    // 画面消去
    window.clear();

    // シェーダプログラムの使用開始
    simple.use();
    simple.loadMatrix(window.getMp(), window.getMw());
    simple.setLight(light);
    simple.setMaterial(material);

    // テクスチャ
    glUniform1i(colorLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    sensor.getColor();
    glUniform1i(depthLoc, 1);
    glActiveTexture(GL_TEXTURE1);
    sensor.getDepth();

    // 図形描画
    sensor.draw();

    // バッファを入れ替える
    window.swapBuffers();
  }
}
