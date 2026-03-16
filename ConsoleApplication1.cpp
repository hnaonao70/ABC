#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
// うんちっちなのだ。はっはっは！！

// dfsa
#include <iostream>
#include <deque>

// ｆｄさｆｄさｆｄさｆｄｄふぁ

#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

// 定数
int unnnnnnn = 4;
const int N = 200;
const int N2 = N * N;
const double TIME_LIMIT = 2.98; // 制限時間3.0秒に対して少し余裕を持たせる

int A[N][N];

// 座標構造体
struct Point
{
    int y, x;
};

Point path[N2];        // 現在の訪問経路
int pos_in_path[N][N]; // マス(y, x)が経路の何番目か(逆引き用)

// 8方向の移動ベクトル
int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};

// 超高速な乱数生成器 (XorShift)
inline uint32_t xor128()
{
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}

// 0 から 1 の小数を返す
inline double rand_double()
{
    return (double)xor128() / 4294967295.0;
}

// 時間管理用タイマー
struct Timer
{
    chrono::high_resolution_clock::time_point start_time;
    Timer() { start_time = chrono::high_resolution_clock::now(); }
    double elapsed() const
    {
        auto now = chrono::high_resolution_clock::now();
        return chrono::duration<double>(now - start_time).count();
    }
};

// 2つのマスが8方向隣接しているか判定
inline bool is_adjacent(const Point &p1, const Point &p2)
{
    return max(abs(p1.y - p2.y), abs(p1.x - p2.x)) <= 1;
}

// 初期解の生成 (単純なジグザグ経路)
void generate_initial_solution()
{
    int idx = 0;
    for (int i = 0; i < N; ++i)
    {
        if (i % 2 == 0)
        { // 右へ
            for (int j = 0; j < N; ++j)
            {
                path[idx] = {i, j};
                pos_in_path[i][j] = idx;
                idx++;
            }
        }
        else
        { // 左へ
            for (int j = N - 1; j >= 0; --j)
            {
                path[idx] = {i, j};
                pos_in_path[i][j] = idx;
                idx++;
            }
        }
    }
}

void solve()
{
    Timer timer;
    generate_initial_solution();

    // 焼きなましの温度設定 (スコア変動量に合わせて要調整)
    double T0 = 20000.0;
    double T1 = 100.0;

    long long iter_count = 0;

    while (true)
    {
        if ((iter_count & 1023) == 0)
        { // 1024回ごとに時間チェック
            double elapsed = timer.elapsed();
            if (elapsed > TIME_LIMIT)
                break;
        }
        iter_count++;

        // 1. パス上のランダムな点 u を選ぶ (両端付近は避ける)
        int u = xor128() % (N2 - 3);
        Point p_a = path[u];

        // 2. p_a の8方向隣接マスからランダムに1つ選び、それを p_b (インデックス v) とする
        int dir = xor128() % 8;
        int ny = p_a.y + dy[dir];
        int nx = p_a.x + dx[dir];

        // 盤面外ならスキップ
        if (ny < 0 || ny >= N || nx < 0 || nx >= N)
            continue;

        int v = pos_in_path[ny][nx];

        // u の次の点 (u+1) よりも後ろで、かつパスの終点でない必要がある
        if (v <= u + 1 || v >= N2 - 1)
            continue;

        // 3. 反転した際のもう一つの接続端点 (u+1 と v+1) が8方向隣接しているかチェック
        Point p_u1 = path[u + 1];
        Point p_v1 = path[v + 1];
        if (!is_adjacent(p_u1, p_v1))
            continue;

        // 【高速化の工夫】長すぎる区間の反転は計算が重いため、短い区間にバイアスをかける
        int length = v - (u + 1);
        if (length > 500 && (xor128() % 10) != 0)
            continue;

        // 4. スコアの差分計算 O(length)
        int L = u + 1;
        int R = v;
        long long delta = 0;

        // Σ (L - R + 2k) * A[P[R-k]] による差分計算
        for (int k = 0; k <= R - L; ++k)
        {
            Point p = path[R - k];
            delta += 1LL * (L - R + 2 * k) * A[p.y][p.x];
        }

        // 5. 焼きなまし法による採用判定
        double elapsed_ratio = timer.elapsed() / TIME_LIMIT;
        double current_temp = T0 * pow(T1 / T0, elapsed_ratio); // 指数冷却

        // スコアが上がる(delta > 0)か、確率的に許容されれば遷移を適用
        if (delta >= 0 || exp(delta / current_temp) > rand_double())
        {
            // パスの区間を反転
            reverse(path + L, path + R + 1);
            // 逆引き用配列を更新
            for (int i = L; i <= R; ++i)
            {
                pos_in_path[path[i].y][path[i].x] = i;
            }
        }
    }

    // 最後に結果を出力 (デバッグ用にイテレーション数を標準エラー出力に出す)
    cerr << "Iterations: " << iter_count << endl;
    for (int i = 0; i < N2; ++i)
    {
        cout << path[i].y << " " << path[i].x << "\n";
    }
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int dummy_N;
    if (!(cin >> dummy_N))
        return 0;

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            cin >> A[i][j];
        }
    }

    solve();

    return 0;
}

// ローカル変更テスト
