# mmball
マウス ボタンの入れ替えとトラック ボールでのスクロール エミュレーションを行います

## できること
* マウス ボタンを入れ替える
* トラック ボールでスクロールをエミュレーションする
* カーソルの移動量曲線を設定する
* 以上の設定をデバイスごとに行う

## 動作環境
* Windows 10 x64 で動作確認をしています．

## 使い方
* プログラムの動作設定はすべてコマンド ライン引数で行います．
* `mmball.exe` を引数なしで実行すると説明が表示されます．
* プログラムを終了する場合はタスク マネージャーからプロセスを終了するか，`mmball.exe` に `/K` オプションを付けて実行します．
* `mmball.exe` は管理者権限実行することをおすすめします（管理者権限のプログラムにフォーカスが移ったときにも正常に動作させるため）

## ビルド方法
* Visual Studio 2017 以降で mmball.sln を開いてビルドします．
* MinGW を使う場合は `g++ --input-charset=utf-8 --exec-charset=cp932 mmball.cpp -mwindows -static` でビルドします．

## リリース履歴

* 2020/05/05 1.1.0 ペンタブなどの絶対座標入力を無視しないよう修正
* 2019/12/22 1.0.0 初版
