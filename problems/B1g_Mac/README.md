# B1g_Mac

> Here's a [zip file](b1g_mac.zip). You can also find the file in `/problems/b1g-mac_0_ac4b0dbedcd3b0f0097a5f056e04f97a`.

Inside the given zip file, we find a windows binary `main.exe` and a folder named `test` with 18 bitmap images inside. We suspect that the images contain some sort of data encoded by `main.exe`, so our first goal is to reverse this binary. Ghidra was quite useful in this process, and we can make out for the most part what the program is doing from it alone. However, it fails to identify certain functions (calling them `.text`), so we can also use Ollydbg to figure out what these function are (`sprintf`, `printf`, etc.).
```c
bool _isOver;
int _pLevel;
char _folderName [8];

int _main() {
	char buf [50];
	FILE* flagfile;
	
	_isOver = 0;
	_pLevel = 0;
	flagfile = fopen("flag.txt","r");
	if (flagfile == NULL) {
		puts("No flag found, please make sure this is run on the server");
	}
	if (fread(buf,1,0x12,flagfile) < 1) {
		exit(0);
	}
	_folderName = "./test"
	_flag = buf;
	_flag_size = 0x12;
	int i = 0;
	_flag_index = &i;
	puts("Work is done!");
	_listdir(0,_folderName);
	puts("Wait for 5 seconds to exit.");
	sleep(5);
	return 2;
}

void _listdir(int mode, char* folderName) {
	char buf [2048];
	_WIN32_FIND_DATAA file_data;
	HANDLE file_handle;
	file_handle = NULL;
	bool shouldAct = true;
	sprintf(buf, "%s\\*.*", folderName);
	file_handle = FindFirstFileA(buf, &file_data);
	if (file_handle == -1) {
		printf("Path not found: [%s]\n", folderName);
	} else {
		do {
			if (strcmp(file_data.cFileName, ".") != 0 && strcmp(file_data.cFileName, "..") != 0) {
				sprintf(buf,"%s\\%s",folderName,file_data.cFileName);
				if ((file_data.dwFileAttributes & 0x10) == 0) {
					if (shouldAct) {
						// Note: mode is always 0, so _hideInFile is always called
						if (mode == 0) {
							_hideInFile(buf);
						} else if (mode == 1) {
							_decodeBytes(buf);
						}
					}
					shouldAct = !shouldAct;
				} else {
					printf("Folder: %s\n",buf);
					hideindir(mode,buf);
				}
			}
			if (_isOver) break;
			FindClose(file_handle);
		} while (FindNextFileA(file_handle,&file_data));
	} 
}

void _hideInFile(char* filename) {
	LPFILETIME creation_time, last_access_time, last_write_time;
	char tmp1, tmp2;
	HANDLE file_handle;
	
	file_handle = CreateFileA(filename,0x100,0,(LPSECURITY_ATTRIBUTES)0x0,3,0,(HANDLE)0x0);
	changeFileTime(file_handle);
	if (file_handle == -1) {
		printf("Error:INVALID_HANDLED_VALUE");
	} else {
		if (!GetFileTime(file_handle,&creation_time,&last_access_time,&last_write_time)) {
			printf("Error: C-GFT-01");
		} else {
			tmp1 = _flag[*_flag_index];
			(*_flag_index) ++;
			tmp2 = _flag[*_flag_index];
			(*_flag_index) ++;
			_encodeBytes(tmp1, tmp2, &last_write_time);
			// Note: _pLevel is always 0, so the below can two blocks have no effect
			if (0 < _pLevel) {
				tmp1 = _flag[*_flag_index];
				(*_flag_index) ++;
				tmp2 = _flag[*_flag_index];
				(*_flag_index) ++;
				_encodeBytes(tmp1, tmp2, &creation_time);
			}
			if (_pLevel == 2) {
				tmp1 = _flag[*_flag_index];
				(*_flag_index) ++;
				tmp2 = _flag[*_flag_index];
				(*_flag_index) ++;
				_encodeBytes(tmp1, tmp2, &last_access_time);
			}
			if (!SetFileTime(file_handle,&creation_time,&last_access_time,&last_write_time)) {
				printf("Error: C-SFT-01");
			}
			else {
				if (_flag_size <= *_flag_index) {
					_isOver = true;
				}
				CloseHandle(file_handle);
			}
		}
	}
}
void _encodeBytes(char c1, char c2, unsigned int* target) {
	*target = (*target & 0xffff0000) + c2 + c1*0x100;
}
void changeFileTime(HANDLE file) {
	SetFileTime(file,NULL,_ftLeaveUnchanged,NULL);
}

void _decodeBytes(char* filename) {
	// we don't actually need this function
}
```
There are several Windows API functions that we might not be familiar with: [`FindFirstFileA`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea), [`FindNextFileA`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilea), [`CreateFileA`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea), [`CloseHandle`](https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle), [`GetFileTime`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfiletime), and [`SetFileTime`](https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfiletime). The first two allow one to iterate through files in a directory. The third and fourth are essentially eqiuvalent to `fopen` and `fclose`, but return a type `HANDLE`, which allows for other I/O operations to be performed. In particular, `GetFileTime` and `SetFileTime` allow (via a `HANDLE` type) the reading and writing of a file's creation time, last access time, and last write time. We suspect that the data is encoded into these file times.

Reading the code confirms our suspicion. For each file in the directory `./test` (actually, every other file; thus, only the `'Copy'` files are modified), two bytes of ASCII are written into the least significant two bytes of the last write time. We also should note the datatype being used, [`FILETIME`](https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime). This datatype represents the number of `100` nanosecond intervals since January 1, 1601 UTC, also known as the [LDAP format](https://www.epochconverter.com/ldap).

To actually extract this data, we have to be careful not to inadvertantly change this. On Windows, we can use `7-zip` to open and extract these files without modifying their file times. Actually viewing the times to the precision we need is quite an annoyance, however. The Windows `Properties` dialogue rounds to the nearest minute. `7-zip` rounds to the nearest second. But we need to see microsecond-level precision. To solve this, we can use the `stat` command in the [Cygwin](https://cygwin.com/) shell. (There are certainly [other ways](https://superuser.com/a/937401/) to get this information, but this one worked simply and quickly). For instance:

```
$ stat 'Item 01 - Copy.bmp'
...
Modify: 2019-03-25 18:20:08.002775300 -0500
...
```
If we convert this into an integer in the LDAP format, we get `131980296080027753`, or `0x1d4e36149337069`. Aha! Those last two bytes are `pi` in ASCII. We continue this process on all of the `'Copy'` files in order to get our flag:

> `picoCTF{M4cTim35!}`