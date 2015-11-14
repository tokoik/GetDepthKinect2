#pragma once

//
// メッシュ
//

// 図形描画
#include "Shape.h"

class Mesh : public Shape
{
  // 描画に使う頂点バッファオブジェクト
  GLuint vbo[3];

  // メッシュの幅
  const GLsizei slices;
  
  // メッシュの高さ
  const GLsizei stacks;
  
  // データとして保持する頂点数
  const GLsizei vertices;
  
  // 実際に描画する頂点数
  const GLsizei indexes;
  
  // カラーデータのテクスチャ座標の読み取り元
  const GLfloat (*texcoord)[2];
  
  // テクスチャ座標を生成してバインドされているバッファオブジェクトに転送する
  void genCoord();

public:

  // コンストラクタ
  Mesh(int stacks, int slices, const GLfloat (*texcoord)[2] = NULL);

  // デストラクタ
  virtual ~Mesh();

  // カラーデータのテクスチャ座標の読み取り元を設定する
  void setTexcoord(GLfloat (*coord)[2])
  {
    texcoord = coord;
  }

  // 描画
  virtual void draw() const;
};
