Answers:
1. 将F12转义成转义字符序列 [ [ L , 对F1-F12处理类似 [ [ A  ->  [ [ L
2. 实现了文件输出的过滤，该过滤是通过修改fs/file_dev.c中file_write（）函数，实现代码类似tty_write（）函数。
   具体修改如下：
		while (c-->0)
		{
			tmp = get_fs_byte(buf++);
			if(f12_flag == 1)
			{
				if((tmp>='A'&&tmp<='Z')||(tmp>='a'&&tmp<='z')||(tmp>='0'&&tmp<='9'))
							tmp = '*';
			}
			*(p++) = tmp;
		}
   如果只过滤终端输出字符，则可以去掉file_write()修改即可；
