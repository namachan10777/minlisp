#include <stdio.h>
#include <gc.h>
#include <node.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("ファイルを一つだけ渡してください\n");
		return -1;
	}
	printf("ファイル名 : %s", argv[1]);
}
