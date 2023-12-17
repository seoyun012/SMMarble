//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"



//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;

typedef struct player {
   int energy;
   int position;
   char name[MAX_CHARNAME];
   int accumCredit;
   int flag_graduate;
} player_t;

static player_t *cur_player;
//static player_t cur_player[MAX_PLAYER]
#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static int player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
#endif

void printGrades(int player)
{
   int i;
   void *gradePtr;
   for(i=0; i<smmdb_len(LISTNO_OFFSET_GRADE + player); i++)
   {
      gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
      printf("%s : %i\n", smmObj_getNodeName(gradePtr), smmObj_getNodeGrade(gradePtr));
   }
}

void printPlayerStatus(void)
{
   int i;
   
   printf("\n<This turn player status>\n"); //player status 사이 간격이 없어 번잡해 보여서 만듦  
   
   for(i=0; i<player_nr; i++)
   {
      printf("%s : credit %i, energy %i, position %i\n", cur_player[i].name, cur_player[i].accumCredit, cur_player[i].energy, cur_player[i].position);
   }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
   int i;
   //n time loop
   for (i=0; i<n; i++)
   {
      //input name
      printf("Input player %i's name:", i);
      scanf("%s", cur_player[i].name);
      fflush(stdin);
      
      //set position
      //player_position[i] = 0;
      cur_player[i].position = 0;
      //set energy
      //player_position[i] = initEnergy;
      cur_player[i].energy = initEnergy;
      cur_player[i].accumCredit = 0;
      cur_player[i].flag_graduate =0;
   }
   
}


int rolldie(int player)
{
    char c;
    printf("\n Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);
    
#if 1
    if (c == 'g')
        printGrades(player);
#endif
    
    return (rand()%MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
   void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position );
    //int type = smmObj_getNodeType( cur_player[player].position );
    int type = smmObj_getNodeType( boardPtr );
    char *name = smmObj_getNodeName( boardPtr );
    void *gradePtr;

   
    switch(type)
    {
        //case lecture1:
        case SMMNODE_TYPE_LECTURE:
           if (cur_player[player].energy >= smmObj_getNodeEnergy(boardPtr)) //현 에너지가 소요 에너지 이상이고 전에 강의 들은 적 없을 때  
            { 
			    cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
                cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);
                     
                //grade generation
                gradePtr = smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
                smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
			}
            else
			printf("Current energy is not enough. Take this lecture next time.\n"); //에너지가 부족해 들을 수 없을 때 출력  
			      
            break;
        
        
		//case laboratory:
	    case SMMNODE_TYPE_LABORATORY:
		   if
		   (cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr)); //실험실에서는 에너지가 사용됨 
		   
		   break;
		
		//case restaurant:
	    case SMMNODE_TYPE_RESTAURANT:
		   if
		   (cur_player[player].energy += smmObj_getNodeEnergy(boardPtr)); //식당, 카페에서는 에너지가 충전됨  
		   
		   break;
		
		//case home:
	    case SMMNODE_TYPE_HOME:
	       if
		   (cur_player[player].energy += smmObj_getNodeEnergy(boardPtr)); //집에서는 에너지가 충전됨  
		   
		   break;
		   
		//case gotolab:
		case SMMNODE_TYPE_GOTOLAB:
		    if 
			(cur_player[player].position = SMMNODE_TYPE_LABORATORY); //실험실로 이동 -> Node 2로 이동해서 수정필요함  
		    printf("%s가 실험실로 이동했습니다.\n", cur_player[player].name); //실험실로 이동했다는 메세지 출력
    
		   break; 
		         
        default:
            break;
    }
}

void goForward(int player, int step)
{
   void *boardPtr;
   cur_player[player].position += step;
   boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
   
   printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
}



int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn = 0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL) 
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH); 
        getchar();
        return -1; 
    }
    
    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i", &name, &type, &credit, &energy) == 4) //read a node parameter set
    {
        //store the parameter set
         //(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
        void *boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);
        
        if (type == SMMNODE_TYPE_HOME)
         initEnergy = energy;
      board_nr++;
    }
    fclose(fp); 
    printf("Total number of board nodes : %i\n", board_nr);
    
    for (i=0; i<board_nr; i++)
    {
       void *boardObj = smmdb_getData(LISTNO_NODE, i);
      printf("node %i : %s, %i(%s), credit %i, energy %i\n", i,  smmObj_getNodeName(boardObj), smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)), smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    
    printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
}
//printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
    
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL) //음식 파일을 읽기 모드로 연다 
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH); //파일 열리지 않으면 오류메세지 출력  
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", &name, &energy) == 2) //read a food parameter set
    {
    	void *foodObj = smmObj_genObject(name, smmObjType_card, 0, 0, energy, 0); //읽은 parameter set 사용해 foodObj 생성 
        smmdb_addTail(LISTNO_FOODCARD, foodObj); //foodObj를 LISTNO_FOODCARD에 추가  
        

		food_nr++;
        //store the parameter set
    }
    fclose(fp); //음식 카드 파일 닫기 
    printf("Total number of food cards : %i\n", food_nr); //음식카드 총 개수  
    
    //각 음식 카드 정보 표시  
    for (i=0; i<food_nr; i++)
    {
       void *foodObj = smmdb_getData(LISTNO_FOODCARD, i); 
      printf("node %i : %s, energy %i\n", i,  smmObj_getNodeName(foodObj), smmObj_getNodeEnergy(foodObj)); //음식 카드엔 정보가 이름과 에너지 둘 뿐  
    
    //SMMNODE_TYPE_FOODCHANCE에 대한 유형 이름 표시  
    printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_FOODCHANCE));
}
   
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL) //페스티벌 카드를 읽기 모드로  연다  
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH); //파일 열리지 않으면 오류메세지 출력  
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", &name) == 1) //read a festival card string
    {
    	void *festObj = smmObj_genObject(name, smmObjType_card, 0, 0, 0, 0); //읽은 parameter set 사용해 festObj 생성 
        smmdb_addTail(LISTNO_FESTCARD, festObj); //festObj를 LISTNO_FESTCARD에 추가 
		
		festival_nr++;
    
        //store the parameter set
    }
    fclose(fp); //페스티벌 카드 파일 닫기
    printf("Total number of festival cards : %i\n", festival_nr); //페스티벌 카드 총 개수  
    
    //각 페스티벌 카드 정보 표시 
     for (i=0; i<festival_nr; i++)
    {
       void *festObj = smmdb_getData(LISTNO_FESTCARD, i); 
      printf("node %i : %s\n", i,  smmObj_getNodeName(festObj)); //페스티벌  카드엔 정보가 미션이름 뿐  
    
    //SMMNODE_TYPE_FESTIVAL에 대한 유형 이름 표시  
    printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_FESTIVAL));
}
   
    
    //2. Player configuration ---------------------------------------------------------------------------------
 
    
    do
    {
        //input player number to player_nr
        printf("input player no.:");
        scanf("%d", &player_nr);
        
        printf("\n"); //간격 위해  
        fflush(stdin);
    }
    while (player_nr < 0 || player_nr > MAX_PLAYER);
    
    cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    
    
  
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);

      //4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1)%player_nr;    
    }
    
    free(cur_player);
    system("PAUSE");
    return 0;
}
