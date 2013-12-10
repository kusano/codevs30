# coding: utf-8
# サドンデスモード用テーブルを作成

W, H = 15, 13

F = [['.']*W for y in range(H)]
for y in range(H):
    F[y][0] = F[y][W-1] = ' '
for x in range(W):
    F[0][x] = F[H-1][x] = ' '
for y in range(2,H-2,2):
    for x in range(2,W-2,2):
        F[y][x] = '#'
F = [F[i/W][i%W] for i in range(W*H)]

D = [1, W, -1, -W]
cp = 1+W
cd = 0
T = []
while True:
    if F[cp]=='.':
        T += [cp]
    F[cp] = ' '
    if F[cp+D[cd]]==' ':
        cd = (cd+1)%4
    if F[cp+D[cd]]==' ':
        break
    cp += D[cd]

A = []
for i in range(500):
    if i<300 or i%2!=0:
        A += [-1]
    else:
        A += T[0:1]
        del T[0]
print ",".join("%3d"%a for a in A)
