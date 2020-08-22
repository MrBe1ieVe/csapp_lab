# CSAPP lab

## datalab

正数用**原码**表示的，负数用**补码**表示

> 补码：正数的补码就是其原码；**负数的反码+1**就是补码
>
> 如单字节5，在计算机中就表示为：0000 0101。负数用补码表示，如单字节-5，在计算机中表示为1111 1011。

### allOddBits

```c++
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int ALL_A = 0xAAAAAAAA;//10101010101010101010101010101010
  return !((x & ALL_A) ^ ALL_A);
}
```



### isAsciiDigit

```c++
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  // x > 0x30 && x < 0x39
  int TMIN = 0x80000000; // 10000000000000000000000000000000
  int minus0x30 = ~0x30 + 1;
  return !((x + minus0x30) & TMIN) // x - 0x30 > 0
    			&& !((0x39 - x) & TMIN); // 0x39 - x > 0
}
```

### conditional x ? y : z

```c++
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(3,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int x1 = !!x;//if x==0, x1 = 1 else x1 = 0
  x = ~x1+1;//if x1 == 1, x = 111111111...11 else x =0
  return (x&y)|(~x&z);
  //if x =11111... 1111 ,(x & y) = y,( ~x & z) = 0
	//if x = 0 , (x & γ〕 = 0, (~x & z) = z
}
```

### isLessOrEqual

```c++
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y)
{
  int isLessOrEqual(int x, int y) {
  int diff_sign = ((x >> 31) & 1)         // x < 0
                  & (((y >> 31) & 1) ^ 1);// y > 0
  int same_sign = (((x >> 31) & 1)        // same equals 1
                  ^ ((y >> 31) & 1) ^ 1); // not the same euqals 0
  int x_minus_y = (((x + (~y)) >> 31) & 1);
  return diff_sign || ( same_sign && x_minus_y);
}
}
```

#### WP

x <= y 的条件等价于

- x，y异号。`sign(x) == 1 && sing(y) == 0`，即`sign(x) & (sign(y) ^ 1)`

    > Sign(x) {
    >
    > ​	return ((x >> 31) & 1); // 逻辑位移，详见下
    >
    > }
    >
    > 如果为负数，返回1
    >
    > 正数，返回0

- x，y同号。

```
1.sign(x) == sign(y) => (sign(x) ^ sign(y) ^ 1)
2.x - y <= 0
=> x - y < 1
=> x + (~y+1) < 1
=> x + (~y) < 1
=> sign(x + (~y))
```



> [来源&参考-最大数值](https://www.cnblogs.com/sinkinben/p/12323784.html)

#### 逻辑位移算数位移

逻辑右移只将二进制数整体右移，**左边补0**，屏蔽了右移带出来的符号位

算术位移，左边补上符号位。