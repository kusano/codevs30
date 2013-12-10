#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <numeric>

using namespace std;



#define DUMP    0
#define FINAL   1



//  フィールドのマス
enum BLOCK
{
    HARD        = 0,    //  ハードブロック
    SOFT        = 1,    //  ソフトブロック
    MAGIC       = 2,    //  魔方陣
    MAGICN      = 3,    //  このターンに設置した魔方陣（キャラ通行可）
    FLOOR       = 4,    //  床
    ITEM_NUM    = 5,    //  アイテム（設置数アップ）
    ITEM_POW    = 6,    //  アイテム（火力アップ）
    ITEM_UNKNOWN= 7,    //  アイテム（不明）
};

//  キャラクタ
struct CHARA
{
    int     pid;        //  プレイヤID
    int     cid;        //  キャラクタID
    int     pos;        //  位置
    int     fire;       //  火力
    int     maxmagic;   //  最大魔方陣数
    int     magic;      //  現在の魔方陣数
};

//  指手
struct MOVE
{
    int     move;       //  移動
    int     magic;      //  魔方陣ターン、0ならば設置しない

    MOVE();
    MOVE(int mv, int mg) : move(mv), magic(mg) {}
    string  toString() const;
};

const int   W   = 15;   //  フィールド幅
const int   H   = 13;   //  フィールド高さ
const int   S   = W*H;  //  フィールド面積
const int   Dir[] = { -1, -W, 1, +W, 0 };
const int   INF = 2000000000;

class CField
{
public:
    int     turn;       //  ターン数
    char    F[S];       //  床面
    CHARA   C[4];       //  キャラクタ　0, 1が自キャラ　2, 3が敵キャラ
    int     Mt[S];      //  魔方陣残りターン数
    int     Mf[S];      //  魔方陣火力

    //  状態保存用
    vector<int>             logT;
    vector<vector<char>>    logF;
    vector<vector<CHARA>>   logC;
    vector<vector<int>>     logMt;
    vector<vector<int>>     logMf;

    int     distEnemy2[S];      //  敵キャラからの距離
    int     distEnemy3[S];      //  敵キャラからの距離
    int     distItem[S];        //  アイテムからの距離

    bool    read(istream &s);
    void    calcDist(int dist[S]) const;
    void    pushState();
    void    popState();
    bool    isValidMove(int cid, MOVE move) const;
    void    move(int cid, MOVE mvoe);
    void    stepMagic(unsigned char A[4][S]);
    void    stepChara(unsigned char A[S]) const;
    string  toString() const;

    //  場所pをキャラクターが通れるか
    bool    passChara(int p) const { return (int)F[p]>=3; }
    //  場所pを雷撃が通れるか
    bool    passMagic(int p) const { return (int)F[p]>=2; }
};
