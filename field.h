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



//  �t�B�[���h�̃}�X
enum BLOCK
{
    HARD        = 0,    //  �n�[�h�u���b�N
    SOFT        = 1,    //  �\�t�g�u���b�N
    MAGIC       = 2,    //  �����w
    MAGICN      = 3,    //  ���̃^�[���ɐݒu���������w�i�L�����ʍs�j
    FLOOR       = 4,    //  ��
    ITEM_NUM    = 5,    //  �A�C�e���i�ݒu���A�b�v�j
    ITEM_POW    = 6,    //  �A�C�e���i�ἨA�b�v�j
    ITEM_UNKNOWN= 7,    //  �A�C�e���i�s���j
};

//  �L�����N�^
struct CHARA
{
    int     pid;        //  �v���C��ID
    int     cid;        //  �L�����N�^ID
    int     pos;        //  �ʒu
    int     fire;       //  �Η�
    int     maxmagic;   //  �ő喂���w��
    int     magic;      //  ���݂̖����w��
};

//  �w��
struct MOVE
{
    int     move;       //  �ړ�
    int     magic;      //  �����w�^�[���A0�Ȃ�ΐݒu���Ȃ�

    MOVE();
    MOVE(int mv, int mg) : move(mv), magic(mg) {}
    string  toString() const;
};

const int   W   = 15;   //  �t�B�[���h��
const int   H   = 13;   //  �t�B�[���h����
const int   S   = W*H;  //  �t�B�[���h�ʐ�
const int   Dir[] = { -1, -W, 1, +W, 0 };
const int   INF = 2000000000;

class CField
{
public:
    int     turn;       //  �^�[����
    char    F[S];       //  ����
    CHARA   C[4];       //  �L�����N�^�@0, 1�����L�����@2, 3���G�L����
    int     Mt[S];      //  �����w�c��^�[����
    int     Mf[S];      //  �����w�Η�

    //  ��ԕۑ��p
    vector<int>             logT;
    vector<vector<char>>    logF;
    vector<vector<CHARA>>   logC;
    vector<vector<int>>     logMt;
    vector<vector<int>>     logMf;

    int     distEnemy2[S];      //  �G�L��������̋���
    int     distEnemy3[S];      //  �G�L��������̋���
    int     distItem[S];        //  �A�C�e������̋���

    bool    read(istream &s);
    void    calcDist(int dist[S]) const;
    void    pushState();
    void    popState();
    bool    isValidMove(int cid, MOVE move) const;
    void    move(int cid, MOVE mvoe);
    void    stepMagic(unsigned char A[4][S]);
    void    stepChara(unsigned char A[S]) const;
    string  toString() const;

    //  �ꏊp���L�����N�^�[���ʂ�邩
    bool    passChara(int p) const { return (int)F[p]>=3; }
    //  �ꏊp�𗋌����ʂ�邩
    bool    passMagic(int p) const { return (int)F[p]>=2; }
};
