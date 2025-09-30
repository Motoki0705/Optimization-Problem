# 最適化フレームワークのアーキテクチャ概要

このドキュメントでは、教育用途およびプロトタイピング向けに設計された軽量な最適化フレームワークの全体構造と設計思想を説明します。リポジトリは複数のトピックを扱えるように再編され、各トピックは独立した C++ ライブラリと実行ファイルを提供します。本ドキュメントでは主に `topics/gradient_descent/` に含まれる勾配降下モジュールの構成について説明します。

## 1. モジュール全体像

```
ユーザーコード / examples
    ↓
Trainer (gd::Trainer)
    ↓
Optimizer (gd::GradientDescentOptimizer)
    ↓
Objective (gd::Objective 派生クラス)
    ↓
Callbacks (gd::Callback 派生クラス) が各イテレーションでフック
```

アプリケーションコードは `gd::Objective` を継承したクラスで目的関数を定義し、`gd::Trainer` に初期解とオプティマイザ設定 (`gd::OptimConfig`) を渡します。トレーナーはループを管理し、必要に応じて有限差分で勾配を近似します。登録された `gd::Callback` には各イテレーションの状態 (`gd::TrainerState`) が通知され、学習率スケジューリングやログ記録、早期終了などを実現できます。

## 2. 主なクラスと責務

### 2.1 `gd::Objective`

* 最適化対象の抽象クラス。次元数と有限差分ステップ幅を保持します。
* `value()` を純粋仮想関数として定義し、派生クラスが目的関数値を計算します。
* `hasAnalyticGradient()` と `analyticGradient()` をオーバーライドすることで解析的勾配を提供できます。未提供の場合は中心差分による数値勾配が用いられます。

### 2.2 `gd::OptimConfig`

* 学習率、収束判定のしきい値、最大反復回数、有限差分ステップなどのハイパーパラメータを保持します。
* `applyDefaults()` で不正または未設定の値をデフォルトに補正します。

### 2.3 `gd::GradientDescentOptimizer`

* 単純な勾配降下の更新則 `x ← x − α ∇f(x)` を担当します。
* 今後別方式のオプティマイザを追加する際の差し替えポイントです。

### 2.4 `gd::Trainer` と `gd::TrainStats`

* 最適化ループを駆動し、勾配の無限大ノルムを評価して収束・停止条件を判断します。
* 実行結果は `gd::TrainStats` として返却され、反復回数・最終目的関数値・最終勾配ノルム・収束フラグなどを含みます。

### 2.5 コールバック (`gd::Callback`)

* 抽象クラス `gd::Callback::onIteration()` を実装することで任意のフック処理を追加できます。
* `gd::CsvLogger` は各イテレーションの状態を CSV へ書き出し、`gd::ConsoleLogger` は標準出力に進捗を表示します。
* `gd::LearningRateDecay` は周期的に学習率を減衰させ、`gd::EarlyStop` は目的関数値が目標を下回ったタイミングで停止フラグを立てます。

## 3. ディレクトリ構成

```
topics/gradient_descent/
  include/gd/        … 公開ヘッダー (`gradient_descent.hpp`)
  src/               … 実装 (`gradient_descent.cpp`)
  examples/          … C++ のサンプルプログラム
scripts/run_gradient_descent.sh … ビルドとデモ実行用ヘルパー
```

サンプルとして `gd1d.cpp`（一次元多項式）と `gd2d.cpp`（二次関数）があり、`gd::Objective` を継承したクラスで目的関数と解析的勾配を実装しています。各サンプルは `gd::CsvLogger` を利用してイテレーション履歴を CSV に書き出し、可視化や解析を容易にします。

## 4. トレーニングループの流れ

1. 現在のパラメータ `x` で `Objective::value()` を評価。
2. `Objective::gradient()` を呼び出し、解析的勾配または有限差分による数値勾配を取得。
3. `gd::TrainerState` を構築し、登録されたすべてのコールバックを実行。
4. 勾配の無限大ノルムが `OptimConfig::tolerance` を下回れば収束とみなして終了。
5. それ以外の場合は `GradientDescentOptimizer::step()` によりパラメータを更新し、次の反復へ移行。

## 5. 拡張ポイント

* **Objective の拡張**: 制約付き最適化や確率的目的関数に対応するため、新しい派生クラスを実装できます。
* **オプティマイザの差し替え**: モメンタム法や Adam などの派生オプティマイザを追加し、`Trainer` に注入できるようにすることで、より高度な手法を実験できます。
* **コールバックの追加**: 進捗を可視化ツールへ送信する、条件付きでハイパーパラメータを変更する、といった機能をコールバックとして実装できます。

## 6. Simplex トピック概要

`topics/simplex/` には線形計画問題 `maximize c^T x` を解くための単体法実装が含まれています。

* `simplex::Problem` で問題の係数を表現し、`simplex::SimplexSolver::solve()` が `simplex::Solution` を返します。
* 状態列挙体 `simplex::Status` は解が「最適」「非有限」「実行不能」「入力エラー」のどれであるかを示します。
* CLI (`simplex_cli.cpp`) は簡潔なテキスト形式を読み込み、解の有無を標準出力へ報告します。

## 7. ビルドと実行

```
# 勾配降下デモ
scripts/run_gradient_descent.sh --example 1d

# 線形計画のデモ
scripts/run_simplex.sh
```

ビルド成果物は `build/topics/<topic>/` 以下に配置され、サンプル実行時には CSV や結果が `topics/<topic>/examples/outputs/` に保存されます。

以上が本フレームワークのアーキテクチャ概要です。各トピックは C++ のオブジェクト指向設計を採用しており、新しい最適化手法やデモを追加する際は既存のクラス設計を参考に構築してください。
