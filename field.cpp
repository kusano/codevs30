#include "field.h"
#include <string>

using namespace std;



//  読み込み
//  読み込みに成功したかどうかを返す
bool CField::read(istream &s)
{
    int dummy;

    //  残り時間
#if FINAL
    s >> dummy;
#endif

    //  ターン
    s >> turn;
    s >> dummy;     //  最大ターン数（1000/500）

    //  プレイヤーID
    int pid;
    s >> pid;

    //  フィールド
    s >> dummy >> dummy;    //  高さ・幅
    for (int y=0; y<H; y++)
    {
        string tmp;
        s >> tmp;
        for (int x=0; x<W; x++)
        {
            switch (tmp[x])
            {
            case '#': F[y*W+x] = HARD;   break;
            case '+': F[y*W+x] = SOFT;   break;
            case '.': F[y*W+x] = FLOOR;  break;
            case '@': F[y*W+x] = FLOOR;  break;
            }
        }
    }

    //  キャラクター
    int cnum;
    s >> cnum;
    for (int i=0; i<cnum; i++)
    {
        //  cid=0,1がpid=0のキャラ、cid=2,3がpid=1のキャラと仮定する
        CHARA c;
        int x, y;
        s >> c.pid >> c.cid >> y >> x >> c.fire >> c.maxmagic;
        c.pos = y*W+x;
        c.magic = c.maxmagic;
        //  常に0を自キャラにする
        c.pid ^= pid;
        c.cid ^= pid<<1;
        C[c.cid] = c;
    }

    //  魔方陣
    for (int i=0; i<S; i++)
        Mt[i] = 9999,
        Mf[i] = 0;
    int mnum;
    s >> mnum;
    for (int i=0; i<mnum; i++)
    {
        int cid, x, y, t, f;
        s >> cid >> y >> x >> t >> f;
        cid ^= pid<<1;
        int p = y*W+x;
        F [p] = MAGIC;
        Mt[p] = min(Mt[p], t);
        Mf[p] = max(Mf[p], f);
        for (int j=0; j<4; j++)
        if (C[j].cid == cid)
            C[j].magic--;
    }

    //  アイテム
    int inum;
    s >> inum;
    for (int i=0; i<inum; i++)
    {
        string type;
        int x, y;
        s >> type >> y >> x;
        F[y*W+x] = type=="NUMBER_UP" ? ITEM_NUM : ITEM_POW;
    }

    string end;
    s >> end;

    if (end!="END")
        return false;

    //  敵キャラとの距離を計算
    for (int i=0; i<S; i++)
        distEnemy2[i] = INF,
        distEnemy3[i] = INF;
    distEnemy2[C[2].pos] = 0;
    distEnemy3[C[3].pos] = 0;
    calcDist(distEnemy2);
    calcDist(distEnemy3);

    //  アイテムとの距離を計算
    for (int i=0; i<S; i++)
        distItem[i] = F[i]==ITEM_NUM || F[i]==ITEM_POW ? 0 : INF;

    calcDist(distItem);

    return true;
}



//  0が設定されているマスからの距離を計算する
//  それ以外のマスはINFに初期化されていること
void CField::calcDist(int dist[S]) const
{
    priority_queue<pair<int, int>> Q;
    bool F[S] = {};

    for (int i=0; i<S; i++)
        if (dist[i]==0)
            Q.push(make_pair(0,i));

    while(!Q.empty())
    {
        int p = Q.top().second;
        Q.pop();
        if(!F[p])
        {
            F[p] = true;
            for (int i=0; i<4; i++)
            {
                int t = p + Dir[i];
                if (passChara(t) || this->F[t]==SOFT || this->F[t]==MAGIC)
                {
                    int d = dist[p] + (passChara(t) ? 1 : 100);
                    if (d < dist[t])
                        dist[t] = d,
                        Q.push(make_pair(-d, t));
                }
            }
        }
    }
}



//  状態を保存
void CField::pushState()
{
    logT .push_back(turn);
    logF .push_back(vector<char> (F,  F +S));
    logC .push_back(vector<CHARA>(C,  C +4));
    logMt.push_back(vector<int>  (Mt, Mt+S));
    logMf.push_back(vector<int>  (Mf, Mf+S));
}



//  状態を復帰
void CField::popState()
{
    turn = logT.back();
    memcpy(F,  &logF .back()[0], sizeof F );
    memcpy(C,  &logC .back()[0], sizeof C );
    memcpy(Mt, &logMt.back()[0], sizeof Mt);
    memcpy(Mf, &logMf.back()[0], sizeof Mf);

    logT .pop_back();
    logF .pop_back();
    logC .pop_back();
    logMt.pop_back();
    logMf.pop_back();
}



//  キャラcidの動きとして正しいか
bool CField::isValidMove(int cid, MOVE move) const
{
    //  通行可能で、魔方陣を設置なら残弾がある
    return (Dir[move.move]==0 || passChara(C[cid].pos+Dir[move.move])) &&
           (move.magic==0 || C[cid].magic>0);
}



//  キャラcidを移動
void CField::move(int cid, MOVE move)
{
    C[cid].pos += Dir[move.move];

    if (move.magic>0)
    {
        int p = C[cid].pos;
        if (F[p]!=MAGIC)
            F[p] = MAGICN;
        Mt[p] = min(Mt[p], move.magic);
        Mf[p] = max(Mf[p], C[cid].fire);
    }
}



//  魔方陣を1ターン進める
//  Aの爆風が通った場所を0にする
void CField::stepMagic(unsigned char A[4][S])
{
    //  爆発をシミュレート
    int M[S];       //  発動する魔方陣
    int mn = 0;     //  個数

    for (int i=0; i<S; i++)
        if ((F[i]==MAGIC || F[i]==MAGICN) && Mt[i]==0)
            M[mn++] = i;

    if (mn>0)
    {
        //  壊れるブロック
        bool B[S] = {};

        while (mn>0)
        {
            int p = M[--mn];
            F[p] = FLOOR;

            for (int d=0; d<4; d++)
            {
                int t = p;
                for (int i=0; i<=Mf[p]; i++, t+=Dir[d])
                {
                    if (F[t]==SOFT)
                        B[t] = true;
                    if (F[t]==MAGIC || F[t]==MAGICN)
                        M[mn++] = t;
                    for (int j=0; j<4; j++)
                        A[j][t] = 0;
                    if (!passMagic(t))
                        break;
                }
            }
        }

        for (int i=0; i<S; i++)
            if (B[i])
                F[i] = ITEM_UNKNOWN;
    }

    //  残りターン数を減らす
    for (int i=0; i<S; i++)
    {
        if (F[i]==MAGIC || F[i]==MAGICN)
            Mt[i]--;
        if (F[i]==MAGICN)
            F[i] = MAGIC;
    }

#if FINAL
    //  サドンデス
    static const int S[500] = {
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         16, -1, 17, -1, 18, -1, 19, -1, 20, -1, 21, -1, 22, -1, 23, -1, 24, -1, 25, -1, 26, -1, 27, -1, 28, -1, 43, -1, 58, -1, 73, -1, 88, -1,103, -1,118, -1,133, -1,148, -1,163, -1,178, -1,177, -1,176, -1,175, -1,174, -1,173, -1,172, -1,171, -1,170, -1,169, -1,168, -1,167, -1,166, -1,151, -1,136, -1,121, -1,106, -1, 91, -1, 76, -1, 61, -1, 46, -1, 31, -1, 33, -1, 35, -1, 37, -1, 39, -1, 41, -1, 57, -1,
         87, -1,117, -1,147, -1,161, -1,159, -1,157, -1,155, -1,153, -1,137, -1,107, -1, 77, -1, 47, -1, 48, -1, 49, -1, 50, -1, 51, -1, 52, -1, 53, -1, 54, -1, 55, -1, 56, -1, 71, -1, 86, -1,101, -1,116, -1,131, -1,146, -1,145, -1,144, -1,143, -1,142, -1,141, -1,140, -1,139, -1,138, -1,123, -1,108, -1, 93, -1, 78, -1, 63, -1, 65, -1, 67, -1, 69, -1, 85, -1,115, -1,129, -1,127, -1,125, -1,109, -1, 79, -1,
    };
    turn++;
    if (turn<500 && S[turn]>=0)
    {
        F[S[turn]] = HARD;
        for (int i=0; i<4; i++)
            A[i][S[turn]] = 0;
    }
#endif
}



//  Aが1の場所にキャラクタがいた場合に、次のステップに移動できる場所を1にする
void CField::stepChara(unsigned char A[S]) const
{
    for (int j=0; j<S; j++)
    if (A[j]==1)
        for (int d=0; d<4; d++)
        {
            int t = j+Dir[d];
            if (A[t]==0 && passChara(t))
                A[t] = 2;
        }

    for (int j=0; j<S; j++)
        if (A[j]==2)
            A[j] = 1;
}



//  盤面を文字列に変換
string CField::toString() const
{
    //  盤面
    string ret = "";
    for (int y=0; y<H; y++)
    {
        for (int x=0; x<W; x++)
        {
            int p = y*W+x;
            if (p==C[0].pos || p==C[1].pos)
                ret += "S";
            else if (p==C[2].pos || p==C[3].pos)
                ret += "E";
            else
                ret += "#+@@.NP?"[(int)F[p]];
        }
        ret += "\n";
    }

    //  敵キャラとの距離
    //for (int y=0; y<H; y++)
    //{
    //  for (int x=0; x<W; x++)
    //      if (distEnemy[y*W+x]<INF)
    //      {
    //          char tmp[16];
    //          sprintf_s(tmp, sizeof tmp, "%4d", distEnemy[y*W+x]);
    //          ret += tmp;
    //      }
    //      else
    //          ret += " ***";
    //  ret += "\n";
    //}

    return ret;
}



//  指手を文字列に変換
string  MOVE::toString() const
{
    string m;

    const char *d[] = {
        "LEFT",
        "UP",
        "RIGHT",
        "DOWN",
        "NONE",
    };
    m += d[move];

    if (magic > 0)
    {
        char tmp[16];
        sprintf_s(tmp, sizeof tmp, " MAGIC %d", magic);
        m += tmp;
    }

    return m;
}
