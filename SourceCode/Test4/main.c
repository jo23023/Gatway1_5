#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#define SCRIPT_LEN  1024
#define SCRIPT_KEYWORD  "---Gateway FW Update Script---"
#define OUTPUT_FILENAME "ceres"
#define MAIN_FILENAME "ceres2"
#define OUTPUT_FOLDER "sn98601"


/* Script Commands
update:PATH/filename,filesize -> update file, same filename
kill:filename -> kill process
copy:filename1, size, PATH/filename2 -> copy filename1 to filename2, can be different filename
del:PATH/filename -> delete files
mkdir:PATH -> make directory
rmdir:PATH -> remove directory, include files inside
run:PATH/filename -> execute process
PS1:the first line should be SCRIPT_KEYWORD
PS2:the first update file should be ceres
PS3:must rename ceres to ceres2 before package (the output file is ceres)
*/

int main()
{
    printf("SN98601 package tool\n");
	struct stat st;

    char script_buf[SCRIPT_LEN];
    char script_buf3[SCRIPT_LEN];

    memset(script_buf, 0, SCRIPT_LEN);
    memset(script_buf3, 0, SCRIPT_LEN);

    //handle script file
	FILE *fp;
	fp = fopen("./update_script", "rb");
	if (fp == NULL)
	{
		printf("No script file found, exit\n");
		return -1;
	}
	int bread = fread(script_buf3, 1, SCRIPT_LEN, fp);
	fclose(fp);
	if(memcmp(script_buf3, SCRIPT_KEYWORD, strlen(SCRIPT_KEYWORD)) != 0)
	{
		printf("Keyword not found, exit\n");
		return -1;
	}
	memcpy(script_buf, script_buf3, SCRIPT_LEN);

	//srtok
	char cmd[1024];
	sprintf(cmd, "rm %s", OUTPUT_FILENAME);
	system(cmd);
	char chars[] = "\n";
	char *p = NULL;
	char *c1, *c2, *c3, *c4, *c5;
	char filename[256];
	p = strtok(script_buf, chars);
	while(p)
	{
	    printf("line=%s\n", p);
        memset(filename, 0, 256);
	    //filename[0] = 0;
	    if(strstr(p, "update:"))
	    {//update command
            c1 = strrchr(p, '/');
            if(!c1)
                c1 = strchr(p, ':');
            c2 = NULL;
            if(c1)
                c2 = strchr(c1, ',');
			//printf("size=[%s]\n",c2+1);
            if(c1 && c2)
            {
                if((c2-c1) > 2)
                {
                    memcpy(filename, c1+1, c2-c1-1);
                }
            }
			// Jeff Add for checking file size 
            if(strcmp(filename, OUTPUT_FILENAME) == 0)
            	{
				stat(MAIN_FILENAME, &st);
				printf("st.st_size= %d\n",(int)st.st_size);
				if(st.st_size != atoi(c2+1))
					{
						printf("!!!File [%s] size error !!!!!!\n",MAIN_FILENAME);
						remove(OUTPUT_FILENAME);				
						exit(0);
					}
            	}
			else
				{
				stat(filename, &st);
				if(st.st_size != atoi(c2+1))
					{
						printf("!!!File [%s] size error !!!!!!\n",filename);
						remove(OUTPUT_FILENAME);
						
						exit(0);
					}
            	}
	    }else if(strstr(p, "copy:"))
	    {//copy command
            c1 = strchr(p, ':');
            c2 = NULL;
            if(c1)
                c2 = strchr(c1, ',');
            if(c1 && c2)
            {
                if((c2-c1) > 2)
                {
                    memcpy(filename, c1+1, c2-c1-1);
                }
            }
	    }
        if(strlen(filename) > 0)
        {
            printf("filename=%s\n", filename);
            memset(cmd, 0, 1024);
            if(strcmp(filename, OUTPUT_FILENAME) == 0)
                sprintf(cmd, "cat %s2 >> %s", filename, OUTPUT_FILENAME);
            else
                sprintf(cmd, "cat %s >> %s", filename, OUTPUT_FILENAME);
            system(cmd);
        }

	    p = strtok(NULL, chars);
	}
	sync();

    //append script file
	FILE *fp2;
	sprintf(filename, "./%s", OUTPUT_FILENAME);
	fp2 = fopen(filename, "ab");
	if (fp2 == NULL)
	{
		printf("No script file2 found, exit\n");
		return -1;
	}
	fwrite(script_buf3, 1, SCRIPT_LEN, fp2);
	fclose(fp2);
	sync();

    //decompress
    // Jeff modify  decompress to temp folder
#if 0
    char script_buf2[SCRIPT_LEN];
	FILE *fp3;
	sprintf(filename, "./%s", OUTPUT_FILENAME);
	fp3 = fopen(filename, "rb");
	if (fp3 == NULL)
	{
		printf("No ceres found, exit\n");
		return -1;
	}
	fseek(fp3, -1024, SEEK_END);
    memset(script_buf2, 0, SCRIPT_LEN);
	bread = fread(script_buf2, 1, SCRIPT_LEN, fp3);
	if(memcmp(script_buf2, SCRIPT_KEYWORD, strlen(SCRIPT_KEYWORD)) != 0)
	{
		printf("Keyword not found, exit\n");
        fclose(fp3);
		return -1;
	}
	rewind(fp3);

	//srtok2
	int filesize = 0;
	int index_now = 0;
	char pathname[256];
	char chars2[] = "\n";
	char filename2[256], filename3[256];
	int i;
	char cc[32];
	char replace_str[64];
	int call_update = 0;
	strcpy(replace_str, "./");
	char *p2 = strtok(script_buf2, chars2);
	while(p2)
	{
	    printf("line=%s\n", p2);
        memset(filename, 0, 256);
	    c3 = strchr(p2, ':');
	    if(c3)
	    {
	        memset(cmd, 0, sizeof(cmd));
	        memcpy(cmd, p2, c3-p2);
	        if(strlen(cmd) > 2)
	        {
                printf("cmd=%s\n", cmd);
                if(strcmp(cmd, "update") == 0)
                {//update
                    c4 = strrchr(p2, '/');
                    if(c4)
                    {//with path
                        memset(pathname, 0, sizeof(pathname));
                        memcpy(pathname, c3+1, c4-c3-1);
                        if(strlen(pathname) > 0)
                        {
                            printf("pathname=%s\n", pathname);
                        }
                        c5 = strchr(c4, ',');
                        if(c5)
                        {
                            memset(filename, 0, sizeof(filename));
                            memcpy(filename, c4+1, c5-c4-1);
                            filesize = atoi(c5+1);
                        }else
                        {
                            filename[0] = 0;
                            filesize = 0;
                        }
                        printf("filename=%s\n", filename);
                        printf("filesize=%d\n", filesize);
                    }else
                    {//without path
                        pathname[0] = 0;
                        printf("pathname=%s\n", pathname);
                        c5 = strrchr(p2, ',');
                        if(c5)
                        {
                            memset(filename, 0, sizeof(filename));
                            memcpy(filename, c3+1, c5-c3-1);
                            filesize = atoi(c5+1);
                        }else
                        {
                            filename[0] =0;
                            filesize = 0;
                        }
                        printf("filename=%s\n", filename);
                        printf("filesize=%d\n", filesize);
                    }
                    if( (strlen(filename) > 0) && (filesize > 0) )
                    {
                        //replace sdcard
                        sprintf(filename2, "%s/%s.u2", pathname, filename);
                        if(strstr(filename2, "sdcard"))
                        {//replace sdcard
                            c1 = strstr(filename2, "sdcard");
                            sprintf(filename3, "%s%s", replace_str, c1+7);
                            strcpy(filename2, filename3);
                        }
                        //del *.u2
                        sprintf(filename3, "rm %s", filename2);
                        system(filename3);
                        FILE *fp4;
                        fp4 = fopen(filename2, "wb");
                        if (fp4 == NULL)
                        {
                            printf("Create file %s failed, exit\n", filename2);
                            return -1;
                        }
                        //write file
                        for(i=0;i<filesize;i++)
                        {
                            fread(cc, 1, 1, fp3);
                            fwrite(cc, 1, 1, fp4);
                        }
                        fclose(fp4);
                        sync();
                        //replace file
                        if(strcmp(filename, "ceres") == 0)
                        {//ceres, rename ceresnew first, kill gateway_wdt, call update, exit ceres
                            system("rm ./ceresnew.old");
                            system("cp ceresnew ceresnew.old");
                            system("cp ceres.u2 ceresnew");
                            system("killall -9 gateway_wdt");
                            call_update = 1;
                        }else
                        {//other file, rename old file, copy new file
                            sprintf(filename3, "killall -9 %s", filename);
                            system(filename3);
                            sprintf(filename3, "rm %s.old", filename);
                            system(filename3);
                            sprintf(filename3, "cp %s %s.old", filename, filename);
                            system(filename3);
                            sprintf(filename3, "rm %s", filename);
                            system(filename3);
                            sprintf(filename3, "cp %s %s", filename2, filename);
                            system(filename3);
                        }
                    }
                }else if(strcmp(cmd, "kill") == 0)
                {//kill
                    //filename[0] = 0;
                    sprintf(filename3, "killall -9 %s", c3+1);
                    //system(filename3);
                }else if(strcmp(cmd, "copy") == 0)
                {//copy
                }else if(strcmp(cmd, "del") == 0)
                {//del
                }else if(strcmp(cmd, "mkdir") == 0)
                {//mkdir
                }else if(strcmp(cmd, "rmdir") == 0)
                {//rmdir
                }else if(strcmp(cmd, "run") == 0)
                {//run
                }else
                {//not match
                    printf("Error, cmd=%s not found!\n", cmd);
                }
	        }
	    }
	    p2 = strtok(NULL, chars2);
	}

	fclose(fp3);

	if(call_update == 1)
        system("./update&");
#else
struct stat sb;
if (stat(OUTPUT_FOLDER, &sb) == 0 && S_ISDIR(sb.st_mode))
{
	printf("start decompress\n");
}
else
{
	system("mkdir sn98601");
	printf("create [%s] and start decompress\n",OUTPUT_FOLDER);
}


char script_buf2[SCRIPT_LEN];
FILE *fp3;
sprintf(filename, "./%s", OUTPUT_FILENAME);
fp3 = fopen(filename, "rb");
if (fp3 == NULL)
{
	printf("No ceres found, exit\n");
	return -1;
}
fseek(fp3, -1024, SEEK_END);
memset(script_buf2, 0, SCRIPT_LEN);
bread = fread(script_buf2, 1, SCRIPT_LEN, fp3);
if(memcmp(script_buf2, SCRIPT_KEYWORD, strlen(SCRIPT_KEYWORD)) != 0)
{
	printf("Keyword not found, exit\n");
	fclose(fp3);
	return -1;
}
rewind(fp3);

//srtok2
int filesize = 0;
int index_now = 0;
char pathname[256];
char chars2[] = "\n";
char filename2[256], filename3[256], filename4[256];
int i;
char cc[32];
char replace_str[64];
int call_update = 0;
strcpy(replace_str, "./sn98601/");
char *p2 = strtok(script_buf2, chars2);
while(p2)
{
	printf("line=%s\n", p2);
	memset(filename, 0, 256);
	c3 = strchr(p2, ':');
	if(c3)
	{
		memset(cmd, 0, sizeof(cmd));
		memcpy(cmd, p2, c3-p2);
		if(strlen(cmd) > 2)
		{
			printf("cmd=%s\n", cmd);
			if(strcmp(cmd, "update") == 0)
			{//update
				c4 = strrchr(p2, '/');
				if(c4)
				{//with path
					memset(pathname, 0, sizeof(pathname));
					memcpy(pathname, c3+1, c4-c3-1);
					if(strlen(pathname) > 0)
					{
						printf("pathname=%s\n", pathname);
					}
					c5 = strchr(c4, ',');
					if(c5)
					{
						memset(filename, 0, sizeof(filename));
						memcpy(filename, c4+1, c5-c4-1);
						filesize = atoi(c5+1);
					}else
					{
						filename[0] = 0;
						filesize = 0;
					}
					printf("filename=%s\n", filename);
					printf("filesize=%d\n", filesize);
				}else
				{//without path
					pathname[0] = 0;
					printf("pathname=%s\n", pathname);
					c5 = strrchr(p2, ',');
					if(c5)
					{
						memset(filename, 0, sizeof(filename));
						memcpy(filename, c3+1, c5-c3-1);
						filesize = atoi(c5+1);
					}else
					{
						filename[0] =0;
						filesize = 0;
					}
					printf("filename=%s\n", filename);
					printf("filesize=%d\n", filesize);
				}
				if( (strlen(filename) > 0) && (filesize > 0) )
				{
					//replace sdcard
					sprintf(filename2, "%s/", pathname);
					if(strstr(filename2, "sdcard"))
					{//replace sdcard
						c1 = strstr(filename2, "sdcard");
						sprintf(filename3, "%s%s", replace_str, c1+7);
						strcpy(filename2, filename3);
						//printf("filename=[%s] filename2=[%s] filename3=[%s]\n",filename,filename2,filename3);
					}
					//del *.u2
					sprintf(filename3, "rm %s", filename2);
					system(filename3);
					FILE *fp4;
					fp4 = fopen(filename2, "wb");
					if (fp4 == NULL)
					{
						printf("Create file %s failed, exit\n", filename2);
						return -1;
					}
					//write file
					for(i=0;i<filesize;i++)
					{
						fread(cc, 1, 1, fp3);
						fwrite(cc, 1, 1, fp4);
					}
					fclose(fp4);
					sync();
					//replace file
					if(strcmp(filename, "ceres") == 0)
					{//ceres, rename ceresnew first, kill gateway_wdt, call update, exit ceres
						system("rm sn98601/ceresnew.old");
						system("cp sn98601/ceresnew sn98601/ceresnew.old");
						system("cp sn98601/ceres.u2 sn98601/ceresnew");
						//system("killall -9 gateway_wdt");
						call_update = 1;
					}else
					{//other file, rename old file, copy new file
						//sprintf(filename3, "killall -9 %s", filename);
						//system(filename3);
						sprintf(filename3, "rm sn98601/%s.old", filename);
						//system(filename3);
						sprintf(filename3, "cp sn98601/%s sn98601/%s.old", filename, filename);
						system(filename3);
						sprintf(filename3, "rm sn98601/%s", filename);
						system(filename3);
						sprintf(filename4, "%s/%s", pathname, filename);
						if(strstr(filename4, "sdcard"))
						{//replace sdcard
							c1 = strstr(filename4, "sdcard");
							sprintf(filename3, "%s%s", replace_str, c1+7);
							strcpy(filename4, filename3);
							printf("filename=[%s] filename3=[%s] filename4=[%s]\n",filename,filename3,filename4);
						}
						sprintf(filename3, "cp %s sn98601s/%s", filename2, filename4);
						system(filename3);
					}
				}
			}else if(strcmp(cmd, "kill") == 0)
			{//kill
				//filename[0] = 0;
				sprintf(filename3, "killall -9 %s", c3+1);
				//system(filename3);
			}else if(strcmp(cmd, "copy") == 0)
			{//copy
			}else if(strcmp(cmd, "del") == 0)
			{//del
			}else if(strcmp(cmd, "mkdir") == 0)
			{//mkdir
			}else if(strcmp(cmd, "rmdir") == 0)
			{//rmdir
			}else if(strcmp(cmd, "run") == 0)
			{//run
			}else
			{//not match
				printf("Error, cmd=%s not found!\n", cmd);
			}
		}
	}
	p2 = strtok(NULL, chars2);
}

fclose(fp3);

//if(call_update == 1)
//	system("./update&");



#endif
    return 0;
}
