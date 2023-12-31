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
   int hasTakenLecture; //이전에 강의 들은 적 있는지 체크하는 상태 변수  
   int experimentTime; //실험 중인지 체크하는 상태 변수  
   int experimentTh; // 실험 성공 기준값 저장 변수  
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
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
#endif


int isGraduated(void)  //check if any player is graduated
{
	int i;

    for (i = 0; i < player_nr; i++)
    {
        // 만약 플레이어 i가 졸업이수학점을 획득한 상황에서 집 노드에 도착하면 졸업  
        if (cur_player[i].accumCredit >= GRADUATE_CREDIT && cur_player[i].position == 0)
        
        {
            printf("%s is graduated! Congratulations!\n" , cur_player[i].name); // 게임 종료 후 졸업한 플레이어 축하 메세지 출력 
            printf("%s's total credit : %s\n", cur_player[i].name, cur_player[i].accumCredit); //졸업한 플레이어의 총 학점 출력 
            printf("Game Over!");
			
            cur_player[i].flag_graduate = 1; //졸업한 플레이어의 flag_graduate 설정
			return i; // i를 반환  
        }
    }
    

    // 모든 플레이어가 아직 졸업 조건을 만족하지 않았다면 
    return -1; 
}


void printGrades(int player)
{
   int i;
   void *gradePtr;
   for(i=0; i<smmdb_len(LISTNO_OFFSET_GRADE + player); i++)
   {
      gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
      printf("%s : %s\n", smmObj_getNodeName(gradePtr), smmObj_getNodeGrade(gradePtr)); //등급을 출력해야하므로 %i -> %s로 변경  
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
      cur_player[i].hasTakenLecture =0; //강의 들은 적 없는 것으로 초기화  
      cur_player[i].experimentTime =0; //실험 중이 아닌 것으로 초기화 
      cur_player[i].experimentTh = rand()%MAX_DIE + 1; // 실험 성공 기준값 초기화
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
        //case lecture:
        case SMMNODE_TYPE_LECTURE:
           if (cur_player[player].energy >= smmObj_getNodeEnergy(boardPtr) && cur_player[player].hasTakenLecture == 0) //현 에너지가 소요 에너지 이상이고 전에 (강의 들은 적 없을 때는 추가해야함)  
            { 
                //강의 수강할지 드랍할지 선택 숫자 1 입력하면 수강, 아니면 드랍  
                int reply;
                printf("If you want to take a lecture, enter number 1 key: ");
                scanf("%i", &reply);
                    //수강을 선택했을 때(숫자 1 입력했을 때)  
                    if(reply == 1)
					{
                      cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr); //학점은 들은 수강과목 학점만큼 쌓임  
                      cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr); //수강하면서 에너지는 소모됨  
                      cur_player[player].hasTakenLecture == 1; //강의 들음 표시
				      
                      //수강 후 완료메세지 출력  
                      printf("The lecture has been completed!! Thank you for your hard work!\n");
                     
                      //grade generation
                      gradePtr = smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
                      smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
                    }
                  
                    else
                    {
                      printf("You dropped this lecture.\n");
                    }
                }
            
			   
		
            else if (cur_player[player].energy < smmObj_getNodeEnergy(boardPtr))
			printf("Current energy is not enough. Take this lecture next time.\n"); //에너지가 부족해 들을 수 없을 때 출력  
			
			else
			printf("You already took this lecture.\n");
			      
            break;
        
        
		//case laboratory:
	    case SMMNODE_TYPE_LABORATORY: {
	    	 
	    	int die_result = rolldie(player); //주사위 굴리고 결과 저장  
	    	 
	    	if (cur_player[player].experimentTime == 1) //실험 중이라면
	    	{
	    		if (die_result >= cur_player[player].experimentTh) //실험 성공이면
				    (cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr)); //실험실에서는 에너지가 사용되고, 바로 이동 
				
				else
				    cur_player[player].position = 8; //실험실노드에 그대로 위치  
			}
			else //실험 중이 아니라면 에너지도 깎이지 않고 바로 이동도 가능  
		       	
			break;
		}


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
		case SMMNODE_TYPE_GOTOLAB: {
			
            printf("Experiment success threshold value: %d\n", cur_player[player].experimentTh);  //실험 성공 기준값을 출력
            
            cur_player[player].experimentTime = 1; //실험 중 상태로 변경 
				
			cur_player[player].position = 8; //전자공학실험실, position 8 로 이동
		    printf("It is 실험시간. %s go to node %i, 전자공학실험실.\n", cur_player[player].name, cur_player[player].position); //실험실로 이동했다는 메세지 출력  
		    
			break;
		}
			
		
		//case foodchance:
		case SMMNODE_TYPE_FOODCHANCE: 
		{
			//음식카드 랜덤 뽑기 
			 int randomFoodCard = rand() % food_nr;
             void *foodCardPtr = smmdb_getData(LISTNO_FOODCARD, randomFoodCard);
             int foodEnergy = smmObj_getNodeEnergy(foodCardPtr);

            //음식 에너지를 현재 에너지에 추가  
            cur_player[player].energy += foodEnergy;
            //플레이어가 음식 카드 뽑은 후의 에너지를 출력 
            printf("%s picked up a food card '%s' and got energy %i. %s's Current energy: %i\n", cur_player[player].name, smmObj_getNodeName(foodCardPtr), foodEnergy, cur_player[player].name, cur_player[player].energy);
            
            break;
	    }
	    
		//case festival:   
		case SMMNODE_TYPE_FESTIVAL: 
		{
			//페스티벌카드 랜덤 뽑기 
			 int randomFestCard = rand() % festival_nr;
             void *festCardPtr = smmdb_getData(LISTNO_FESTCARD, randomFestCard);
            
            //플레이어가 페스티벌 카드 뽑은 후 미션 완료 메세지 출력  
            printf("%s picked up a festival card '%s' and completed mission.\n", cur_player[player].name, smmObj_getNodeName(festCardPtr));
            
            break;
	    }
		         
        default:
            break;
    }
}

void goForward(int player, int step)
{
    int gameEnd = 0;  //게임종료 구분 
	int nodeTotalNum = 16;  // 노드 총 개수  
	void *boardPtr;
	
	while(!gameEnd)  //게임 종료 되기 전까지 반복 
	{
     	cur_player[player].position += step;
     	
     	//플레이어가 총 노드 수 16을 넘기면  
     	if (cur_player[player].position >= nodeTotalNum)
        {   
		    //나머지 연산을 통해 위치를 처음 턴처럼 되돌림   
            cur_player[player].position %= nodeTotalNum;   
        }
        
        boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position );
        
     	printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
					
		if(isGraduated())  //졸업한 플레이어가 존재하면  -> 코드 변경 필요해보임  
		{
			gameEnd = 1;  //while문 끝, 게임 종료되어야  
		} 	
	}                
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
    while (isGraduated() == -1) // isGraduated()코드에서 졸업한 학생이 없으면 -1을 출력하게 했으므로  
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

