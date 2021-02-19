# CTON

[https://github.com/short-circuits/cton](https://github.com/short-circuits/cton)

C言語のためのオブジェクトライブラリ。*コメント：ここのオブジェクトは、オブジェクト指向プログラミングのオブジェクトとは別のものである。JSONのオブジェクト（順序づけされていないキーと値のペアの集まり）とおなじ意味である*。

JSON解析機能実装したため、データ交換が便利。そして独自のバイナリフォーマットが実装してます。JSONより早く解析できで、JSONが表現できないデータでも交換できます。

(cJSONなどと比べ、)メモリプールを利用することができます。想定上の使い方は、一つのクライアントに使うメモリは、全部一つのメモリプールから割り当てます。そしてクライアントとの通信が終わったとき、メモリプールを解放することで、メモリプールから割り当てたすべてのメモリをシステムに返せます。サーバの長持つ環境でもメモリリークが発生しにくいです。

研究になれませんが、研究に便利さを与えるため、ここに載ってます。そしてほかのところでも使ってるから、研究室じゃないアカウントのところに置いてます。

---

*こいつは研究になれるの？*

どちらが日本語ドキュメントを作ってくれたらありがたいです🥺

## みなもと

- The C Object System: Using C as a High-Level Object-Oriented Language ([arXiv](https://arxiv.org/abs/1003.2547), [GitHub](https://github.com/CObjectSystem/COS))
- cJSON: Ultralightweight JSON parser in ANSI C ([GitHub](https://github.com/DaveGamble/cJSON))
- Protocol Buffers - Google's data interchange format ([GitHub](https://github.com/protocolbuffers/protobuf))
- nginx: High Performance Load Balancer, Web Server, & Reverse Proxy ([code](http://hg.nginx.org/nginx/file/default/src/core/ngx_palloc.c))
- A dichromatic framework for balanced trees ([IEEE](https://doi.org/10.1109/SFCS.1978.3))
- NBT: Named Binary Tag specification ([webarchive](https://web.archive.org/web/20110723210920/http://www.minecraft.net/docs/NBT.txt))
- APR: Apache Portable Runtime
- The Programming Language Lua

## 発想

- C言語は早くて、資源の利用が少なくて、システムプログラミングや組込みシステムで使われている。ですが、メモリの管理が面倒だし、バグが発生しやすい。さらに、ハッシュのデータ構造が持ってないため、毎回構造体を作らなければならず、プログラミングの進むが遅い。なので、軽量化のオブジェクトライブラリがほしい。
- nginxというHTTPは、サーバーの環境を考え、メモリリークを防ぐため、メモリプールでメモリを管理する。コネクションごとにメモリプールを作って、このコネクションのためのメモリ確保は全てこのメモリプールから行う。コネクションが切ったら、メモリプールを解放だけで、メモリプールから確保したすべてメモリは全部解放する。このライブラリはエッジサーバなどに使うかもしれないので、このような機能がほしい（cJSONやFastJSONがこの点でアウト）
- 一つの応用を長持つ視点から考え、メモリリークは大きな問題、でもnginxのようなメモリ管理は長持つのアプリに適応ではない。なのであるガベージコレクション機能がほしい。
- エッジサーバーとロボットをC/Sモデルで考え、あるデータ交換方法が必要である。JSONは一つのやり方であるので、こもようなライブラリは両方でも使える。しかし、JSON仕様による、すべても数値型はdoubleである。つまりint64_tのようなデータを転送したいでも、JSONはそのまま使うことはできない。なので、あるデータ構造も提案したい。

## 目標

- 基本のデータ構造が表現できるC言語用軽量オブジェクトライブラリ。
    - ❌C言語に対する改造はしない。インタプリタを組み込むこともしない。軽量という目標を忘れず、APRのようなものにならない。
    - 基本データ構造とは、32ビット符号あり整数や64ビット符号なし整数など、C言語が持つ、かつハードウェアが処理できるデータ型以外、配列やハッシュなどプログミング言語でよく使うものが含む、最低限でもJSONとNBTが表現できるデータ型が処理できる。
    - APIはできるだけシンプルにする（KISS）、オブジェクトマネジメント以外の機能はユーザに任せる。
- メモリプールが利用でき、ガベージコレクション機能も実装する。
- TON (Toaru Object Notation)とTBON (Toaru Binary Object Notation)二つのデータ交換ようフォーマットを提案する。この二つもフォーマットは、ライブラリが表現できるすべてのデータ型を必ず記述できる。そしてTBONの方は、直列化と直列化復元のオーバヘッドは小さければ小さいほどよい。
- 想定上の利用場面はこれら：
    - C言語でのアプリサーバー。C言語でアプリサーバーを作るのは、多分CGIかFastCGIのイメージがすぐ頭に浮かぶかもしれないが、考えたことがありますか？nginxをアプリサーバーにする方法？アプリとWebサーバをリンクし、非同期IOを採用し、とても高性能のアプリサーバーができるはずと思う。このサーバーはもしメモリリークかスタックオーバーフローが発生することを抑えるのは、このライブラリだ。
    - ロボットなどの低電力デバイスやメモリの少ないデバイスは、この小ちゃいライブラリにぴったり。
    - C言語でデータをある形で保存したい場合、このライブラリで直列化。

## 評価（もうやっちゃった）

- JSONの解析性能はruby 2.6.4とほぼ同じ。(i7-4558Uに10MのJSONで評価した、CPU過熱などを考えると、結果は厳密ではない)
- 867MB JSONでの実験
    - 実験環境Core i3-10110U@4.1GHz
    - Ruby-mri (best score): 19.090s
    - Pythonでやりたいが、意外にkillされた。
    - cton: 4.516s

## ライブラリ構造

これからライブラリの構造を述べる。

日本語でどうやっていうかわからないが、とりあえず『ユーザー』の言葉でこのライブラリを使っているプログラマーのことを表す。

### CTONコンテキスト

CTONコンテキスト（以下コンテキスト）は、メモリ管理の基本単位です。メモリの管理はメモリフークの形で行う。メモリフークの定義はこのようである：

```c
typedef struct {
    void *      pool;
    void *    (*palloc)(void *pool, size_t size);
    void *    (*prealloc)(void *pool, void *ptr, size_t size);
    void      (*pfree)(void *pool, void *ptr);
    void      (*pdestroy)(void *pool);
} cton_memhook;
```

このストラクチャーは、各コンテキストに保存し、各コンテキストは違うものを指定することが可能です。CTONはメモリを確保するときは、コンテキストに保存したこのストラクチャーの関数でメモリ確保を行う。

メモリフークの指定は、コンテキストの作るときしかできない（メモリフーク変更による予想外のメモリリークを防ぐため）。指定方法はこのようである。

```c
cton_ctx *ctx;
cton_memhook *hook;

hook = cton_memhook_init(pool, palloc, prealloc, pfree, pdestroy);
ctx = cton_init(hook);
```

cton_memhook_initのパラメータは幾つのNULLがあっても構わない。なおメモリフークを利用したくない場合、cton_initにNULLを渡してもok。この場合は、メモリフークは自動的にlibcのmalloc/freeに設定する。

コンテキストはガベージコレクションのためのデータも保存しているが、これはあとで説明する。

### CTONオブジェクト

ちょっと定義が外れたが、ここのオブジェクトはCTONによる管理する、データとその型を持ち、CTONが提供するメソッドで操作できるものである。オブジェクトが持つデータの型（以下オブジェクトの型）は静的で、変更不能です。オブジェクトの作るのはこのようである：

```c
cton_obj *obj;
obj = cton_object_create(ctx, CTON_HASH);
```

このようで、ハッシュのデータ型が持ってるCTONオブジェクトが作った。このobjはメモリフークに指定したメモリプールから確保したため、メモリプールを解放するときは、このobjも解放される。

オブジェクトメソッドはGitHubに参照すればいいと思う。

## ガベージコレクション

想定上、このライブラリはガベージコレクション機能があるはずだが、今まで使う機会がなかったから、実装することは先伸ばしてる。

設計のうえ、CTONコンテキストはルートという特別なオブジェクトポインタがある。それはTracing GCのためのものである。最初のステージは、このルートから、全体のオブジェクトに走査を実行し、到達可能の各オブジェクトを「使用中」としてマークするマークステージである。CTONコンテキストは、すべてのオブジェクトが走査できるリンクリストがある。第二番のステージはこのリンクリストを走査し、使用中のマークが付いてないオブジェクトを回収する。

GCは自動的に行うことではない、ユーザーがcton_gc(ctx)を呼び出したときにだけ、ctxに対してGCを行う。なので、まだルートに追加したかったオブジェクトが意外に回収してしまう心配はない。