//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//
#ifndef smm_object_h
#define smm_object_h

#define SMMNODE_TYPE_LECTURE        0
#define SMMNODE_TYPE_RESTAURANT     1
#define SMMNODE_TYPE_LABORATORY     2
#define SMMNODE_TYPE_HOME           3
#define SMMNODE_TYPE_GOTOLAB        4
#define SMMNODE_TYPE_FOODCHANCE     5
#define SMMNODE_TYPE_FESTIVAL       6

#define SMMNODE_TYPE_MAX            7

typedef enum smmObjType {
    smmObjType_board = 0,
    smmObjType_card,
    smmObjType_grade
} smmObjType_e;   //smmObjType_e, �Ʒ� object generation ���� ���� ���� �̸� ����  

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/


/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-
*/

//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjType_e grade); //smmObjGrade_e grade ���� -> smmObjType_e grade�� ����    
//member retrieving
char* smmObj_getNodeName(void* obj);
int smmObj_getNodeType(void* obj);
int smmObj_getNodeCredit(void* obj);
int smmObj_getNodeEnergy(void* obj);

//element to string
char* smmObj_getTypeName(int type);
char* smmObj_getNodeGrade(int grade);

#endif /* smm_object_h */