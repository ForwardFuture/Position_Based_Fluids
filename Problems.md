## $\bold{Core}$

#### $\text{Part 1: Search of Neighboring Particles}$

使用KDTree或hash grid



#### $\text{Part 2: Density}$

##### $\text{Problem 1.1: Rest Density}$

**采用模型：**粒子紧密排布构成的立方体

**计算方法：**以其中一个粒子的中心为中心，用密度估计函数对静止密度进行估计

##### $\text{Problem 1.2: Density Estimation}$

**对于核函数包括区域未超出 $\bold{Bounding\; Box}$ 的粒子：**使用密度估计函数正常计算

**对于核函数包括区域超出 $\bold{Bounding\;Box}$ 的粒子：**假设 $\text{Bounding Box}$ 外均是紧密排布的虚拟粒子，将核函数包括的虚拟粒子一并加入密度估计函数的计算



#### $\text{Part 3: Collision Detection & Response}$

（复杂度较高）

目前采用的方法是直接枚举点对，通过解方程判断是否碰撞，并将delta_p取min

## $\bold{Problems}$

#### $\text{Problem 1}$

$\bold{Description}$：粒子下落后不会弹起

$\bold{Solution}$：在PBF中根据速度预测粒子位置时加入碰到边界速度分量反向的操作。碰撞反弹后垂直于墙壁的位移分量可以乘以 $\text{damping}$ 因子模拟能量损耗



#### $\text{Problem 2}$

$\bold{Description}$：发现少量粒子获得持续向上的速度

$\bold{Solution}$：进一步观察当粒子周围粒子数较少时均会出现该异常情况，否则均不会。初步认为是预测后的调整阶段delta_p计算出错，导致粒子纵向正向位移较大



Problem Particle:

60 3.01457
80 1.48477
94 1.54493
153 1.69504
198 1.035
253 3.01457
290 3.13484
341 1.45481

## $\bold{And\; then...}$

调整 $\text{Parameters.md}$ 中的参数