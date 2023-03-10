## sudoku

C++实现的终端数独游戏.

![demo](https://bu.dusays.com/2023/03/06/64056316539d1.png)

## 编译 & 运行

```shell
g++ ./main.cpp -o sudoku && ./sudoku
```

## 操作

- 移动: `w/a/s/d`
- 填写: `1-9`, `.`表示留空
- 重置: `r`
- 新游戏: `n`, 生成同等难度的新游戏
- 检查: `c`, 检查所有所填项是否可以解决该题
- 提示: `h`, 提示当前空格可以填入的候选项, 该候选项不保证可解
- 更换难度: `-/=`, `-`表示降低难度, `=`表示提高难度, 难度等级1-7
- 退出: `q`

## 已知问题

- 不保证唯一解
- 偶现题目生成需要很久
