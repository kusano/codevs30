#include "field.h"



void simulate(CField *field, int T, int moveenemy, int a[][4]);
int judge(int a[][4], int T);



int main()
{
    cout << "kusano" << endl;

    CField field;
#if !FINAL
    CField fieldp;
#endif
    int clear = 0;

    while (field.read(cin))
    {
#if !FINAL
        //  クリア数を出力
        if (field.turn>0)
        {
            bool f = false;
            for (int i=0; i<4; i++)
            {
                int t = abs(field.C[i].pos-fieldp.C[i].pos);
                if (t!=0 && t!=1 && t!=W)
                    f = true;
            }
            if (f)
            {
                clear++;
                cerr << "clear stage " << clear
                    << " (turn=" << field.turn-1 << ")" << endl;
            }
        }
        fieldp = field;
#endif

#if DUMP
        cerr << "Field:" << endl;
        cerr << field.toString() << endl;
#endif

        long long mscore = -INF;
        vector<pair<MOVE,MOVE>> move;

        //  自キャラの指手
        //  4方向と、魔方陣設置を設置するかどうかを探索
        for (int dir0=0; dir0<5; dir0++) for (int mt0=0; mt0<=5; mt0+=5)
        for (int dir1=0; dir1<5; dir1++) for (int mt1=0; mt1<=5; mt1+=5)
        {
            int f0 = field.C[0].pos;
            int f1 = field.C[1].pos;

            MOVE mv0(dir0, mt0);
            MOVE mv1(dir1, mt1);

            if (!field.isValidMove(0, mv0) ||
                !field.isValidMove(1, mv1))
                continue;

            field.pushState();
            field.move(0, mv0);
            field.move(1, mv1);

            //  Tターン後の両者の移動可能マス数を求める
            const int T = 16;
            int A[T][4];
            simulate(&field, T, true, A);

            long long score = 0;

            if (field.distEnemy2[f0]<=4 ||
                field.distEnemy2[f1]<=4 ||
                field.distEnemy3[f0]<=4 ||
                field.distEnemy3[f1]<=4)
            {
                //  敵キャラに近い場合は、自分の移動可能範囲を広げ、
                //  敵の範囲を狭める
                score += min(A[T-1][0], A[T-1][1]);
                score -= min(A[T-1][2], A[T-1][3]);
            }
            else
            {
                //  敵キャラから遠い場合は、アイテムもしくは敵に近づく
                if (field.distItem[f0] <= 2)
                    score -= field.distItem[field.C[0].pos];
                else
                    score -= field.distEnemy2[field.C[0].pos];
                if (field.distItem[f1] <= 2)
                    score -= field.distItem[field.C[1].pos];
                else
                    score -= field.distEnemy2[field.C[1].pos];
            }

            //  敵キャラの指手
            //  魔方陣を設置できるなら設置する
            for (int dir2=0; dir2<5; dir2++)
            for (int dir3=0; dir3<5; dir3++)
            {
                int mt2 = field.C[2].magic>0 ? 5 : 0;
                int mt3 = field.C[3].magic>0 ? 5 : 0;

                MOVE mv2(dir2, mt2);
                MOVE mv3(dir3, mt3);

                if (!field.isValidMove(2, mv2) ||
                    !field.isValidMove(3, mv3))
                    continue;

                field.pushState();
                field.move(2, mv2);
                field.move(3, mv3);

                int A[T][4];
                simulate(&field, T, false, A);

                if (judge(A, T)<0)
                    score = -INF+1;

                field.popState();
            }

            //  勝敗が決まるなら±INF
            int res = judge(A, T);
            if (res>0)
                score = INF;
            if (res<0)
                score = -INF;

            field.popState();

#if DUMP
            cerr << mv0.toString() << " " << mv1.toString() << " " << score << endl;
#endif

            if (score>mscore)
            {
                move.clear();
                mscore = score;
            }
            if (score==mscore)
                move.push_back(make_pair(mv0, mv1));
        }

        if (move.empty())
            move.push_back(make_pair(MOVE(4,0), MOVE(4,0)));

        int p = rand()%(int)move.size();
        cout << move[p].first.toString() << endl;
        cout << move[p].second.toString() << endl;
    }
}



//  fieldをTターン進めて各ターンのキャラの移動可能マス数を返す
//  moveenemyが真なら敵キャラを1ターン動かす
void simulate(CField *field, int T, int moveenemy, int a[][4])
{
    field->pushState();

    unsigned char A[4][S] = {};
    for (int i=0; i<4; i++)
        A[i][field->C[i].pos] = 1;

    if (moveenemy)
        field->stepChara(A[2]),
        field->stepChara(A[3]);

    for (int i=0; i<T; i++)
    {
        field->stepMagic(A);
        for (int j=0; j<4; j++)
            field->stepChara(A[j]);
        for (int j=0; j<4; j++)
            a[i][j] = accumulate(A[j], A[j]+S, 0);
    }

    field->popState();
}



//  勝敗判定
//  プレイヤーが勝つなら1、相手が勝つなら-1、その他は0
int judge(int a[][4], int T)
{
    for (int i=0; i<T; i++)
    {
        int s = min(a[i][0], 1) + min(a[i][1], 1);
        int e = min(a[i][2], 1) + min(a[i][3], 1);

        if (s<2 || e<2)
#if FINAL
            return s>e ? 1 : s<e ? -1 : 0;
#else
            return s==2 ? 1 : -1;
#endif
    }
    return 0;
}
