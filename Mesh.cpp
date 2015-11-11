#include "Mesh.h"

//
// メッシュ
//

// テクスチャ座標の生成してバッファオブジェクトに転送する
void Mesh::genTexcoord()
{
  GLfloat (*const t)[2](static_cast<GLfloat(*)[2]>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)));
  for (int i = 0; i < vertices; ++i)
  {
    t[i][0] = (GLfloat(i % slices) + 0.5f) / GLfloat(slices);
    t[i][1] = (GLfloat(i / slices) + 0.5f) / GLfloat(stacks);
  }
  glUnmapBuffer(GL_ARRAY_BUFFER);
}

// コンストラクタ
Mesh::Mesh(int slices, int stacks, const GLfloat (*texcoord)[2])
  : slices(slices)
  , stacks(stacks)
  , vertices(slices * stacks)
  , indexes((slices - 1) * (stacks - 1) * 3 * 2)
  , texcoord(texcoord)
{
  // 描画に使うメッシュデータの頂点バッファオブジェクトを準備する
  glGenBuffers(sizeof vbo / sizeof vbo[0], vbo);

  // デプスデータのサンプリング用のバッファオブジェクト
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices * 2 * sizeof (GLfloat), NULL, GL_STATIC_DRAW);

  // デプスデータのテクスチャ座標を求めてバッファオブジェクトに転送する
  genTexcoord();

  // インデックスが 0 の varying 変数に割り当てる
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // カラーデータをデプスデータの画素単位にサンプリングするためのバッファオブジェクト
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, vertices * 2 * sizeof (GLfloat), NULL, GL_DYNAMIC_DRAW);

  // カラーデータのテクスチャ座標を求めてバッファオブジェクトに転送する
  genTexcoord();

  // インデックスが 1 の varying 変数に割り当てる
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);

  // インデックス用のバッファオブジェクト
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes * sizeof (GLuint), NULL, GL_STATIC_DRAW);

  // インデックスを求めてバッファオブジェクトに転送する
  GLuint *index(static_cast<GLuint *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY)));
  for (int j = 0; j < stacks - 1; ++j)
  {
    for (int i = 0; i < slices - 1; ++i)
    {
      index[0] = slices * j + i;
      index[1] = index[5] = index[0] + 1;
      index[2] = index[4] = index[0] + slices;
      index[3] = index[2] + 1;
      index += 6;
    }
  }
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

// デストラクタ
Mesh::~Mesh()
{
  // 頂点バッファオブジェクトを削除する
  glDeleteBuffers(sizeof vbo / sizeof vbo[0], vbo);
}

// 描画
void Mesh::draw() const
{
  // テクスチャ座標の読み取り先が設定されていれば
  if (texcoord != NULL)
  {
    // カラーのテクスチャ座標をバッファオブジェクトに転送する
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices * 2 * sizeof(GLfloat), texcoord);
  }

  // 頂点配列オブジェクトを指定して描画する
  Shape::draw();
  glDrawElements(GL_TRIANGLES, indexes, GL_UNSIGNED_INT, NULL);
}
