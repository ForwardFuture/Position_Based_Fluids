#### $\text{Adjustable Parameters in Global.h:}$

$\bold{eps}$： 用于预防浮点精度导致问题的参数。默认为 $1e-6$
$\bold{Width}$：视口的宽。默认为 $800$ 像素
$\bold{Height}$：视口的高。默认为 $600$ 像素
$\bold{ROW}$：用于模拟的粒子个数参数，这表示一共有 $\text{ROW}\times \text{ROW}\times \text{ROW}$ 个粒子参与模拟。默认为 $7$
$\bold{radius}$：用于模拟的粒子半径。默认为 $0.015$
$\bold{Yoffset}$：用于模拟的粒子相对于容器底端的偏移。默认为 $0.4$



#### $\text{Adjustable Parameters in PBF.cpp:}$

$\bold{deltaTime}$：模拟的时间步长。默认为 $0.02$
$\bold{MaxIteration}$：PBF中修正粒子预测位置的迭代次数。默认为 $5$
$\bold{MaxNum}$：用于计算核函数半径的一个参数。默认为 $3$
$\bold{damping}$：衰减因子，该因子用于PBF中初次预测粒子位置时粒子与墙壁碰撞后反弹的计算。默认为 $0.7$
$\bold{EPS}$：用于实现PBF中计算 $\lambda$ 的软约束。默认为 $1e-6$
※※※$\bold{Tensile\_k\_for\_particle}$：用于计算粒子间 $s_{corr}$ 的参数。默认为 $0.01$
※※※$\bold{Tensile\_k\_for\_wall}$：用于计算粒子与墙壁间 $s_{corr}$ 的参数。默认为 $0.006$