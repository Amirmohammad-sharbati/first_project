#define _GNU_SOURCE
#include <stdio.h>     
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 2000
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000

#define debug(x) printf("%s", x);


void print_command(int argc, char * const argv[]);

int run_init(int argc, char * const argv[]);
int create_configs(char *cwd);
int global_pwd_neogit(char * cwd);

int go_top_directory(char * cwd);
void local_username(char * username);
void local_email (char * email);
void global_username (char * username);
void global_email (char * email);

char * movestring(char *string , int n);
char *find_path(char *filepath);
int path_stage(char *filepath);
int run_add(int argc, char * const argv[]);
int add_to_staging(char *filepath);
int track_file(char *filepath);
int is_tracked(char *filepath);

int check_file(char *filepath);
int check_staging(char *filepath);
int reset(int argc, char *filepath);
int remove_from_staging(char *filepath);

int last_stage(int argc);
int run_commit(char *message);
int inc_last_commit_ID();
int commit_staged_file(int commit_ID, char *filepath);

char* getCurrentTimeString() ;
int create_commit_file(int commit_ID, char *message);
int find_file_last_commit(char* filepath);



int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("please enter a valid command");
        return 1;
    }
    
    // print_command(argc, argv);

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;

    char *username="-";
    char *email="-";

    if (strcmp(argv[1], "init") == 0) {
        return run_init(argc, argv);
    } 
    
    else if(strcmp(argv[1],"config")==0) {
        if(strcmp(argv[2],"user.name")==0 ){
            username=argv[3];
            local_username(username);
        }
        else if(strcmp(argv[3],"user.name")==0){
            username=argv[4];
            global_username(username);
        }
        else if(strcmp(argv[2],"user.email")==0 ){
            email=argv[3];
            local_email(email);
        }
        else if(strcmp(argv[3],"user.email")==0){
            email=argv[4];
            global_email(email);
        }
    }

     else if (strcmp(argv[1], "add") == 0) {
        if(strcmp(argv[2],"-f")!=0 && strcmp(argv[2],"-n")!=0 && argc>3){
            for(int i=0; i<argc-2; i++){
                if (chdir(cwd) != 0) 
                    return 1;
                char *path=find_path(argv[i+2]);
                if(check_file(path)==5) 
                    add_to_staging(path);  
                else if(check_file(path)==10){
                DIR *dir;
                struct dirent *entry;
                dir = opendir(path);
                if(dir != NULL){
                    while ((entry = readdir(dir)) != NULL) {
                        char path_tmp[1000];
                        strcpy(path_tmp,path);
                        if (entry->d_type == DT_REG) {
                            strcat(path_tmp,"/");
                            strcat(path_tmp,entry->d_name);
                            add_to_staging(path_tmp); 
                        }
                    }
                }
            }
            }
        }

        //-f
        else if(strcmp(argv[2],"-f")==0){
            for(int i=0; i<argc-3; i++){
                if (chdir(cwd) != 0) 
                    return 1;
                char *path=find_path(argv[i+3]);
                if(check_file(path)==5) 
                    add_to_staging(path);  
                else if(check_file(path)==10){
                DIR *dir;
                struct dirent *entry;
                dir = opendir(path);
                if(dir != NULL){
                    while ((entry = readdir(dir)) != NULL) {
                        char path_tmp[1000];
                        strcpy(path_tmp,path);
                        if (entry->d_type == DT_REG) {
                            strcat(path_tmp,"/");
                            strcat(path_tmp,entry->d_name);
                            add_to_staging(path_tmp); 
                        }
                    }
                }
            }
            }
        }

        else {
            char *path=find_path(argv[2]);
            if(check_file(path)==5) 
                add_to_staging(path);  
            else if(check_file(path)==10){
                DIR *dir;
                struct dirent *entry;
                dir = opendir(path);
                if(dir != NULL){
                    while ((entry = readdir(dir)) != NULL) {
                        char path_tmp[1000];
                        strcpy(path_tmp,path);
                        if (entry->d_type == DT_REG) {
                            strcat(path_tmp,"/");
                            strcat(path_tmp,entry->d_name);
                            add_to_staging(path_tmp); 
                        }
                    }
                }
            }
        }
    } 
    
    
    
    else if (strcmp(argv[1], "reset") == 0) {
        if (argc<3) {
            printf("please specify a file\n");  
            return 1;
        }    
        char *path=find_path(argv[2]);
            if(check_file(path)==5) remove_from_staging(path);  
            else if(check_file(path)==10){
                DIR *dir;
                struct dirent *entry;
                dir = opendir(path);
                if(dir != NULL){
                    while ((entry = readdir(dir)) != NULL) {
                        char path_tmp[1000];
                        strcpy(path_tmp,path);
                        if (entry->d_type == DT_REG) {
                            strcat(path_tmp,"/");
                            strcat(path_tmp,entry->d_name);
                            remove_from_staging(path_tmp); 
                        }
                    }
                }
            }
    } 
    
    else if (strcmp(argv[1], "commit" ) == 0 && strcmp(argv[2],"-m")==0) {
        char message[1000];
        strcpy(message,argv[3]);
        if(strlen(message)>72) {   
            printf("characters of Message must be less than 72\n");  
            return 1; 
        }
        run_commit(message);
    } 
    
    return 0;
}

void print_command(int argc, char * const argv[]) {
    for (int i = 0; i < argc; i++) 
        printf("%s ", argv[i]);
    printf("\n"); 
}

// This function is responsible for initializing a "neogit" repository.
int run_init(int argc, char * const argv[]) {
    char cwd[1000];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;
//getcwd function(get current working directory): we can know we are in which directory
    
    char tmp_cwd[1000];
    bool exists = false;
    struct dirent *entry; //we have an struct which moves between directories
    do {
// DIR type represents a directory stream and it's a structure that holds information of open directory.
// This loop traverses the current and parent directories --> find .neogit 
        DIR *dir = opendir("."); // with opendir func we can open the address (if it's directory)
        if (dir == NULL) {
            printf("Error opening current directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".neogit") == 0)
                exists = true;
        }
        closedir(dir);

        // update current working directory
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return 1;

        if (strcmp(tmp_cwd, "/") != 0) {
            if (chdir("..") != 0) return 1;
        }

    } while (strcmp(tmp_cwd, "/") != 0);

    // return to first cwd 
    if (chdir(cwd) != 0) return 1;

    if (exists) 
        printf("neogit repository has already initialized");
    else {
        if (mkdir(".neogit", 0755) != 0){
            printf("Error in making .neogit");
            return 1;
        } 
        return global_pwd_neogit(cwd);
    }
    return 0;
}


int go_top_directory(char * cwd)
{
    do {
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        return 1;

        if (strcmp(cwd, "/mnt/c") != 0) {
// chdir(change to parent directory) gets .. and if there isn't any problem returns 0.
            if (chdir("..") != 0) return 1;
        }
    } while (strcmp(cwd, "/mnt/c") != 0);
}

int global_pwd_neogit(char *cwd)
{
    char wd[1000];

    go_top_directory(wd);
    
    FILE *file = fopen("cwd", "w");
    fprintf(file, "%s", cwd);
    fclose(file);

    return create_configs(cwd);
}

void local_username(char * username)
{
    FILE * file = fopen("username", "w");
    fprintf(file, "%s", username);
    fclose(file);
}

void local_email (char * email) 
{
    FILE *file = fopen("email", "w");
    fprintf(file, "%s", email);
    fclose(file);
}

void global_username (char * username)
{
    char cwd[1000];
    go_top_directory (cwd);
    
    FILE *file = fopen("username", "w");
    fprintf(file, "%s", username);
    fclose(file);
}

void global_email (char * email)
{
    char cwd[1000];
    go_top_directory (cwd);
    
    FILE *file = fopen("email", "w");
    fprintf(file, "%s", email);
    fclose(file);
}

int create_configs(char *cwd) {
    char username[200];
    char email[200];

    char path1[1000];
    strcpy(path1 , cwd);
    strcat(path1,"/username");

    // accessibility of a file or directory in the file system --> access
        if (access(path1, F_OK) == 0){
        FILE *file = fopen(path1, "r");
        fgets(username, sizeof(username), file);
        printf("%s\n",username);
        fclose(file);
    } 
    else if (access("/mnt/c/username", F_OK) != -1){
        FILE *file = fopen("/mnt/c/username", "r");
        fgets(username, sizeof(username), file);
        fclose(file);
    } 

    char path2[1024];
    strcpy(path1,cwd);
    strcat(path1,"/email");
    if (access(path2, F_OK) == 0){
        FILE *file = fopen(path1, "r");
        fgets(email, sizeof(email), file);
        fclose(file);
    } 
    else if (access("/mnt/c/email", F_OK) != -1){
        FILE *file = fopen("/mnt/c/email", "r");
        fgets(email, sizeof(email), file);
        fclose(file);
    } 
    // move to neogit working directory
    if (chdir(cwd) != 0) return 1;

    
    FILE *file = fopen(".neogit/config", "w");
    if (file == NULL) return 1;

    fprintf(file, "username: %s\n", username);
    fprintf(file, "email: %s\n", email);
    fprintf(file, "last_commit_ID: %d\n", 0);
    fprintf(file, "branch: %s", "master");

    fclose(file);
    
    // create commits folder
    if (mkdir(".neogit/commits", 0755) != 0) return 1;

    // create files folder
    if (mkdir(".neogit/files", 0755) != 0) return 1;

    file = fopen(".neogit/staging", "w");
    fclose(file);

    file = fopen(".neogit/tracks", "w");
    fclose(file);

    file=fopen(".neogit/branchs","w");
    fprintf(file, "%s\n","master");
    fclose(file);

    return 0;
}

char * movestring(char *string , int n)
{
    int lenght = strlen(string);
    for(int i=0 ; i<lenght - n ; i++) {
        string[i]=string[i + n];
    }

    string[lenght - n] = '\0';
    return string;
}

int run_add(int argc, char *const argv[]) 
{ 
    if (argc < 3) {
        perror("please specify a file");
        return 1;
    }

    return path_stage(argv[2]);
}

int path_stage(char * filepath){
    
    int count=0;
    while(filepath[0]=='.' && filepath[1]=='.') { // when .. exists in address.
        movestring(filepath , 3);
        count++;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;

    for(int i = strlen(cwd); 1; i--){
        if(cwd[i]=='/') {
            cwd[i]='\0';  
            count--;
            }
        if(count<=0) 
         break;
    } 
    
    char file_path[1024];
    strcpy(file_path,cwd);
    strcat(file_path,"/");
    strcat(file_path,filepath);
    return add_to_staging (file_path);
}

char *find_path(char *filepath)
{
    int count=0;
    while(filepath[0]=='.' && filepath[1]=='.'){
        movestring(filepath,3);
        count++;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
        return "error";

    for(int i=strlen(cwd); 1; i--){
        if(cwd[i]=='/'){  
            cwd[i]='\0';  
            count--;
        }
        if(count<=0) 
         break;
    } 

    char *file_path=(char *)malloc(1000 * sizeof(char));
    strcpy(file_path,cwd);
    strcat(file_path,"/");
    strcat(file_path,filepath);
    return file_path;
}

int add_to_staging(char *filepath) {
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    //going to neogit place
    if (chdir(pwd) != 0)
     return 1;

    if (access(filepath, F_OK) == 0) {
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) 
    return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (line[length - 1] == '\n') 
            line[length - 1] = '\0';

        if (strcmp(filepath, line) == 0) return 0;
    }
    fclose(file);
    
    file = fopen(".neogit/staging","a");
    if (file == NULL) return 1;

    fprintf(file, "%s\n", filepath);
    fclose(file);

    track_file(filepath);

    //find name of file
    char * name=strrchr(filepath,'/');
        if (name!=NULL)
            name++;
        else
            name=filepath;
           
    FILE *sourceFile, *destinationFile;
    char c;

    sourceFile = fopen(filepath, "rb");
        if (sourceFile == NULL) {
            printf("Error opening source file");
            return 1;
        }

        if (chdir(".neogit/staged_files") != 0) return 1;

        destinationFile = fopen(name, "wb");
        if (destinationFile == NULL) {
            printf("Error opening destination file");
            fclose(sourceFile);
            return 1;
        }

        while ((c = fgetc(sourceFile)) != EOF) 
            fputc(c, destinationFile);

        fclose(sourceFile);
        fclose(destinationFile);

            return 0;
        }
    return 1;
}

int track_file(char *filepath) 
{
    if (is_tracked(filepath)) 
        return 0;

    FILE *file = fopen(".neogit/tracks", "a");
    if (file == NULL)
        return 1;
    fprintf(file, "%s\n", filepath);
    return 0;
}

int is_tracked(char *filepath) {
    //find place of neogit
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL)
    return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    if (chdir(pwd) != 0)
        return 1;

    file = fopen(".neogit/tracks", "r");
    if (file == NULL) 
        return 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (line[length - 1] == '\n')
            line[length - 1] = '\0';
        
        if (strcmp(line, filepath) == 0) return 1;

    }
    fclose(file); 

    return 0;
}

int check_file(char *path){
    struct stat fileStat;
    if (stat(path, &fileStat) == 0) {
        if (S_ISREG(fileStat.st_mode))
            return 5;
        else if (S_ISDIR(fileStat.st_mode))
            return 10;
        else {
            printf("It isn't file or directory.\n");
            return 1;
        }
    } else {
        printf("the file(%s) is not existed\n",path);
        return 1;
    }
}

int check_staging(char *filepath)
{
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) 
        return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    if (chdir(pwd) != 0)
         return 1;

    file = fopen(".neogit/staging", "r");
    if (file == NULL) 
        return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL){
        int length = strlen(line);
        if (line[length - 1] == '\n') 
            line[length - 1] = '\0';

        if (strcmp(filepath, line) == 0)
             return 0;
    }

    return 1;
}

int remove_from_staging(char *filepath) 
{
    //find place of neogit
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);


    char *name=strrchr(filepath,'/');
    if(name!=NULL)
        name++;
    else
        name=filepath;  

    if (chdir(pwd) != 0) return 1;

    if (access(filepath, F_OK) == 0){
        FILE *file = fopen(".neogit/staging", "r");
        if (file == NULL) 
            return 1;
    
        FILE *tmp_file = fopen(".neogit/tmp_staging", "w");
        if (tmp_file == NULL) 
            return 1;

        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
            int length = strlen(line);

            if (line[length - 1] == '\n') 
                line[length - 1] = '\0';

            if (strcmp(filepath, line) != 0) {
            fputs(line, tmp_file);
            fputs("\n", tmp_file);
            }
        }
        fclose(file);
        fclose(tmp_file);

        remove(".neogit/staging");
        rename(".neogit/tmp_staging", ".neogit/staging");
        if (chdir(".neogit/stages_files") != 0) return 1; 
        remove(name);
        return 0;
    }
    else{
        return 1;
    }
}

int reset(int argc, char * filepath) 
{
    if (argc<3) {
        printf("please use correct format.");
        return 1;
    }
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) 
        return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

     //find name of file
    char *name=strrchr(filepath,'/');
    if(name!=NULL)
        name++;
    else
        name=filepath;
     
    if (chdir(pwd) != 0)
         return 1;

    if (access(filepath, F_OK) == 0){
        FILE *file = fopen(".neogit/staging", "r");
        if (file == NULL) return 1;
    
        FILE *tmp_file = fopen(".neogit/tmp_staging", "w");
        if (tmp_file == NULL) 
            return 1;

        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
            int length = strlen(line);

            // remove '\n'
            if (line[length - 1] == '\n')
                line[length - 1] = '\0';

            if (strcmp(filepath, line) != 0) {
            fputs(line, tmp_file);
            fputs("\n", tmp_file);
            }
        }
        fclose(file);
        fclose(tmp_file);

        remove(".neogit/staging");
        rename(".neogit/tmp_staging", ".neogit/staging");
        if (chdir(".neogit/staged_files") != 0) 
            return 1; 
        remove(name);
            return 0;
    }
    else
        return 1;

}

int last_stage(int argc)
{
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) 
        return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    if (chdir(pwd) != 0) 
        return 1;
    //find count of line
    int count=0;
    char c;
    file=fopen(".neogit/staging","r");
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n') 
            count++;
    } 
    fclose(file);

    file=fopen(".neogit/staging","r");
    char line[MAX_LINE_LENGTH];
    for (int i=0; i<count; i++) {
        fgets(line, sizeof(line), file);
        int length=strlen(line);
        if (line[length - 1] == '\n')
            line[length - 1] = '\0';
    }
    char last_line[1000];
    strcpy(last_line,line);
    fclose(file);
    return reset(argc, last_line);
}

int run_commit(char *message) 
{    
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) 
        return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    if (chdir(pwd) != 0) return 1;

    //find count of staging file
    int count_of_file=0;
    file = fopen(".neogit/staging", "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        count_of_file++;
    }

    fclose(file);
    if(count_of_file==0){
        printf("no stage file\n");
        return 1;
    }
    //ID
    int commit_id=inc_last_commit_ID();
    //Time
    char *currentTimeStr = getCurrentTimeString();
    
    //write information of commit in file
    FILE *first,*second,*staging;
    char second_path[1000];
    strcpy(second_path,".neogit/commits/");
    char string[100];
    sprintf(string, "%d", commit_id);
    strcat(second_path,string);
    first=fopen(".neogit/config","r");
    second=fopen(second_path,"w");

    int flag=0;
    while (fgets(line, sizeof(line), first) != NULL) {
        fprintf(second, "%s", line);
        flag++;
        if(flag==3)  
            break;
    }

    fclose(first);
    staging=fopen(".neogit/staging","r");
    fprintf(second, "id: %d\n", commit_id);
    fprintf(second, "message: %s\n", message);
    fprintf(second, "Time: %s\n", currentTimeStr);
    fprintf(second, "count of file: %d\n", count_of_file);
    fprintf(second, "%s\n", "adress of files :" );
    while (fgets(line, sizeof(line), staging) != NULL) {
        fprintf(second, "%s", line);
    }
    fclose(second);
    fclose(staging);
    // make folder and copy stages file
    strcat(second_path,"f");
    if (mkdir(second_path, 0755) != 0) 
        return 1;

    staging=fopen(".neogit/staging","r");
    while (fgets(line, sizeof(line), staging) != NULL){
        FILE *sourceFile, *destinationFile;
        char c;

        int length = strlen(line);
        if (line[length - 1] == '\n') 
            line[length - 1] = '\0';
        

        char *name=strrchr(line,'/');
        if(name!=NULL)
            name++;
        else
            name=line;

        char second_pwd[1000];
        strcpy(second_pwd,second_path);
        strcat(second_pwd,"/");
        strcat(second_pwd,name);

        sourceFile = fopen(line, "rb");
        if (sourceFile == NULL) {
            printf("Error opening source file");
            return 1;
        }

        destinationFile = fopen(second_pwd, "wb");
        if (destinationFile == NULL) {
            printf("Error opening destination file");
            fclose(sourceFile);
            return 1;
        }

        while ((c = fgetc(sourceFile)) != EOF) {
            fputc(c, destinationFile);
        }

        fclose(sourceFile);
        fclose(destinationFile);
    }

    fclose(staging);

    staging=fopen(".neogit/staging","w");
    fclose(staging);

    //delete files of staged
    DIR *dir;
    struct dirent *entry;
    dir=opendir(".neogit/staged_files");
    while ((entry = readdir(dir)) != NULL){
        char cwd[1000];
        strcpy(cwd,pwd);
        strcat(cwd,"/.neogit/staged_files/");
        strcat(cwd,entry->d_name);
        if(check_file(cwd)==5){
            remove(cwd);
        }
    }
    //print information of commit
    printf("commit ID:\t%d\n",commit_id);
    printf("commit Time:\t%s\n",currentTimeStr);
    printf("commit Message:\t%s\n",message);
    return 0;
}

// returns new commit_ID
int inc_last_commit_ID() 
{
    FILE *file = fopen("/mnt/c/pwd","r");
    if (file == NULL) 
        return 1;
    char pwd[1000];
    fgets(pwd, sizeof(pwd), file);
    fclose(file);

    //going to neogit place
    if (chdir(pwd) != 0) 
        return 1;

    file = fopen(".neogit/config", "r");
    if (file == NULL) 
        return -1;
    
    FILE *tmp_file = fopen(".neogit/tmp_config", "w");
    if (tmp_file == NULL) 
        return -1;

    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);
            last_commit_ID++;
            fprintf(tmp_file, "last_commit_ID: %d\n", last_commit_ID);

        } else 
            fprintf(tmp_file, "%s", line);
    }

    fclose(file);
    fclose(tmp_file);

    remove(".neogit/config");
    rename(".neogit/tmp_config", ".neogit/config");
    return last_commit_ID;
}

char* getCurrentTimeString() 
{
    time_t currentTime;
    time(&currentTime);

    char *timeString = ctime(&currentTime); //converting to string

    size_t length = strlen(timeString);
    if (timeString[length - 1] == '\n') 
        timeString[length - 1] = '\0';
    
    return timeString;
    //char *currentTimeStr = getCurrentTimeString();
}

