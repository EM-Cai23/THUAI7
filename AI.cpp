#include <vector>
#include <thread>
#include <array>
#include <queue>
#include <map>
#include "AI.h"
#include "constants.h"
#include <math.h>
#include<algorithm>
#define pi 3.14159265358979323846
#define sqr2 1.4142136
// ע�ⲻҪʹ��conio.h��Windows.h�ȷǱ�׼��
// Ϊ����play()�ڼ�ȷ����Ϸ״̬�����£�Ϊ����ֻ��֤��Ϸ״̬�ڵ�����ط���ʱ�����£�����һ֡����һ��
extern const bool asynchronous = true;

// ѡ����Ҫ���ν�player1��player4�Ĵ����������ﶨ��
extern const std::array<THUAI7::ShipType, 4> ShipTypeDict = {
    THUAI7::ShipType::CivilianShip,
    THUAI7::ShipType::CivilianShip,
    THUAI7::ShipType::MilitaryShip,
    THUAI7::ShipType::MilitaryShip,
};

// ������AI.cpp�ڲ����������뺯��

// ��������
const int map_size = 50;
int dx[] = {1, 1, 1, 0, 0, -1, -1, -1};
int dy[] = {-1, 0, 1, 1, -1, 1, 0, -1};
int count1 = 0;
int count2 = 0;
int index_close = -1;
bool temp1 = false;
const double SPEED_CIVIL_MS = 3.00;
const double SPEED_MILIT_MS = 2.80;
const double SPEED_FLAG_MS = 2.70;
std::vector<int> ship_re(10, 0);
int kill_number = 0;
bool Map_State = false;

struct Point
{
    int x, y;
};
struct Point direction[1000];
// �����ĸ��࣬����ִ����ز���
class my_Resource
{
public:
    int HP = 16000;
    int x;
    int y;
    // ���ɵ�λ
    int x_4p;
    int y_4p;
    int x_4p2;
    int y_4p2;

    // �Ƿ��Ѿ����ɹ���
    bool produce = false;
    my_Resource(int x_, int y_, int x4, int y4)
    {
        x = x_;
        y = y_;
        x_4p = x4;
        y_4p = y4;
    }
};

class my_Home
{
public:
    int x;
    int y;
    int HP = 48000;

    // 1���� 2�з�
    int group = 0;

    // �������ڷ����ҵĵ�λ
    int x_4p;
    int y_4p;
    my_Home(int i, int j, int gp)
    {
        x = i;
        y = j;
        group = gp;
    }
    my_Home()
    {
        x = 0;
        y = 0;
    }
};
std::vector<my_Home> home_vec;

class my_Construction
{
public:
    THUAI7::ConstructionType type = THUAI7::ConstructionType::NullConstructionType;
    int x;
    int y;
    int home_dist;

    // �����λ
    int x_4c;
    int y_4c;
    int HP = 0;
    // 0Ϊ�� 1Ϊ���� 2Ϊ�з�
    int group = 0;
    // �½�bool�������Ƿ񱻽���
    bool build = false;
    my_Construction(int i, int j, int i_4c, int j_4c)
    {
        x = i;
        y = j;
        x_4c = i_4c;
        y_4c = j_4c;
        home_dist = ((i - home_vec[0].x) * (i - home_vec[0].x)) + ((j - home_vec[0].y) * (j - home_vec[0].y));
    }
    my_Construction()
    {
        x = 0;
        y = 0;
    }
};

class my_Wormhole
{
public:
    int x;
    int y;
    int HP = 18000;

    my_Wormhole(int i, int j, int hp)
    {
        x = i;
        y = j;
        HP = hp;
    }
};

class my_Enemy
{
public:
    THUAI7::ShipType shiptype;
    int hp;
    int gridx;
    int gridy;
    THUAI7::WeaponType weapon;

    void Update(THUAI7::ShipType ship, int Hp, int x, int y, THUAI7::WeaponType Weapon);
    my_Enemy(THUAI7::ShipType ship, int Hp, int x, int y, THUAI7::WeaponType Weapon)
    {
        shiptype = ship, hp = Hp, gridx = x, gridy = y;
        weapon = Weapon;
    };
    my_Enemy();
};
void my_Enemy::Update(THUAI7::ShipType ship, int Hp, int x, int y, THUAI7::WeaponType Weapon)
{
    shiptype = ship;
    hp = Hp;
    gridx = x;
    gridy = y;
    weapon = Weapon;
    return;
}
bool once_met_enemys=false;

std::vector<my_Enemy> enemy_vec;
std::vector<my_Resource> resource_vec;
std::vector<my_Construction> construction_vec;
std::vector<my_Wormhole> wormhole_vec;
my_Construction closest_2_home;

std::vector<std::vector<int>> Map_grid(map_size, std::vector<int>(map_size, 1));

// �����ǵ��õĺ����б�(Basic)
//void Judge(IShipAPI& api);                         // �ж�Ӧ�������ĸ�����(����/��ȡ��Դ��)
void Base_Operate(ITeamAPI& api);                  // ���صĲ���
void AttackShip(IShipAPI& api);                    // �����з���ֻ
void Install_Module(ITeamAPI& api, int number, int type);  // Ϊ��ֻ��װģ�� 1:Attack 2:Construct 3:Comprehensive
bool GoCell(IShipAPI& api);                        // �ƶ���cell����
bool attack(IShipAPI& api);                                // ���ط���+�жϵ���
void hide(IShipAPI& api);                                  // ˿Ѫ����
const double Count_Angle(IShipAPI& api, int tar_gridx, int tar_girdy);  // ���㷽λ�ĺ���

// JUDGE���������ж�Ӧ������ʲô���͵Ĳ���->ת�ӵ�Attack_Ships/Attack_Cons/Attack_Home
// ͨ����ͬ�ķ���ֵת�ӵ���ͬ�ĺ���

// �����񴬽��о����жϣ������ܹ�������Ϣ�����������ڽ���Эͬ
int Judge_4_Civil(IShipAPI& api);
// ���ڻ��ؽ������
void Judge_4_Base(ITeamAPI& api);

// ���µ�����Ϣ
bool Update_Enemy(IShipAPI& api);
int Enemy_Attack_Index(IShipAPI& api);

// ������Ѱ·��صĺ���
bool isValid(IShipAPI& api, int x, int y);
std::vector<std::vector<int>> Get_Map(IShipAPI& api);                            // �õ�һ����άvector��������ͼ�Ͽ���/��������Ϣ
void Update_Map(IShipAPI& api);                                                 // ����MAP,�����Ѿ��ж�
const std::vector<Point> findShortestPath(const std::vector<std::vector<int>>& grid, Point start, Point end, IShipAPI& api);  // �����������Ѱ·
bool GoPlace(IShipAPI& api, int des_x, int des_y);
bool GoPlace_Loop(IShipAPI& api, int des_x, int des_y);
bool GoPlace_Dis(IShipAPI& api, int des_x, int des_y);  // �ھ�����Զ�ĵط�����
void GoPlace_Dis_Loop(IShipAPI& api, int des_x, int des_y);
bool Path_Release(std::vector<Point> Path, IShipAPI& api, int count);  // ʵ��·��(δʹ��)
void Update_Cons(IShipAPI& api);

// �����ǿ�����Դ��������غ���

// �����������м�����Դ
void Get_Resource(IShipAPI& api);

// ̰���㷨���ɼ�����Դ
void Greedy_Resource(IShipAPI& api);

// ����limit��������Դ��֮���˳�
void Greedy_Resource_Limit(IShipAPI& api, int limit);

// ��ѡ����λ�ý���ѡ���Ľ�����
void Build_Specific(IShipAPI& api, THUAI7::ConstructionType type, int index);

// ������λ�ý���ѡ���Ľ�����
void Build_ALL(IShipAPI& api, THUAI7::ConstructionType type);

// ̰���㷨���콨����
void Greedy_Build(IShipAPI& api, THUAI7::ConstructionType type);

// �ػ��ػ�Ѫ
void Go_Recover(IShipAPI& api);

//  �Ƿ񱻹���
bool Under_Attack(IShipAPI& api);

// �����������ѭ��
bool Attack_Loop_Cons(IShipAPI& api, double angle, my_Construction cons);

// ����������
bool Attack_Cons(IShipAPI& api);

// �����з�����
void Attack_Base(IShipAPI& api);
    // �����Ǵ�Ӫ������غ���

// ��װ����ģ��
void Produce_Module(ITeamAPI& api, int shipno, int type = 3);

// ��װ����ģ��
void Construct_Module(ITeamAPI& api, int shipno, int type = 3);

// ��������
void Build_Ship(ITeamAPI& api, int shipno, int birthdes);

// �����ܺ���
void Base_Build_Ship(ITeamAPI& api, int birthdes);

// ģ���ܺ���
void Base_Module_Install(ITeamAPI& api);

//��װ����ģ��
void Military_Module(ITeamAPI& api, int shipno, int type = 0);
void Military_Module_weapon(ITeamAPI& api, int shipno, int type = 3);
void Military_Module_armour(ITeamAPI& api, int shipno, int type = 3);
void Military_Module_shield(ITeamAPI& api, int shipno, int type = 3);

//��������
void Strategy_Military_Steal(IShipAPI& api);                        //͵��
void Strategy_Military_Guard(IShipAPI& api);                        //�ؼ�
bool Chase(IShipAPI& api,int last_seen_gridx,int last_seen_gridy);  //׷����ʹ��ʱ�����ǰ�����once_met_enemys=false;֮����δ�ӵ�������������Ϊ����attackship/goplaceѭ������

void Resource_Attack(IShipAPI& api);            //(�ӱ�Ķ�ѧ����) �����з�Resource��������

void Construction_Attack(IShipAPI& api);

// ��������(ί���߼���)
bool Advantage(IShipAPI& api);

//�������XX��ͼ����
std::pair<int, int> findclosest(IShipAPI& api,THUAI7::PlaceType type, int des_x, int des_y); 

/*
������ͨ�ŽӿڵĶ��壺

�����䣺��һλ"1" ��ʾ�������� "0"��ʾ����
�ڶ���������λ��ʾ�������ڵ�λ��"0120" Ĭ��ȫΪ0

�����䣺
��һλ��ʾ�����װ������ "0":Sheild "1":Armor "2":Weapon
�ڶ�λ��ʾװ���ı�� "1""2""3"��
��������
1Ĭ��Laser Gun 2ΪPlasma 3ΪShell 4Ϊmissle 5ΪArc

�����䣺
�����������ֶ���ʾ��Ϯ
*/

void Decode_Me_4_Milit(IShipAPI& api);

void Send_Me(IShipAPI& api, std::string str);

void Decode_Me(ITeamAPI& api);

void Send_Me(ITeamAPI& api);

// ��play�������е��õ��񴬺���
void Play_4_Civil(IShipAPI& api)
{
    if (!Map_State)
    {
        Get_Map(api);
    }
    Greedy_Resource_Limit(api, 3);
    Greedy_Build(api, THUAI7::ConstructionType::Factory);
    Greedy_Resource(api);
}

void Play_4_Milit(IShipAPI& api)
{
    if (api.GetSelfInfo()->teamID == 1)
    {
        return;
    }
    if (!Map_State)
    {
        Get_Map(api);
    }
    /*
    if (api.GetSelfInfo()->playerID == 3)
    {
        Decode_Me_4_Milit(api);
        auto place = findclosest(api, THUAI7::PlaceType::Shadow, 25, 23);
        GoPlace_Loop(api, place.first, place.second);
        api.PrintSelfInfo();
        if (kill_number < 2 && !Advantage(api))
        {
            AttackShip(api);
        }
        else
        {
            Construction_Attack(api);
            Resource_Attack(api);
            GoPlace_Loop(api, home_vec[1].x + 2, home_vec[1].y);
            AttackShip(api);
            if (api.GridToCell(api.GetSelfInfo()->x) == home_vec[1].x + 2 && api.GridToCell(api.GetSelfInfo()->y) == home_vec[1].y)
                api.Attack(pi);
        }
    }
    else
    {
        api.PrintSelfInfo();
        Decode_Me_4_Milit(api);
        Attack_Base(api);
        if (Advantage(api))
        {
            Resource_Attack(api);
            Construction_Attack(api);
        }
    }
    */
    GoPlace_Dis_Loop(api, home_vec[1].x + 1, home_vec[1].y);
    Attack_Base(api);
    return;
}

void AI::play(IShipAPI& api)
{
    if (this->playerID == 1)
    {
        Play_4_Civil(api);
    }
    else if (this->playerID == 2)
    {
        Play_4_Civil(api);
    }
    else if (this->playerID == 3)
    {
        Play_4_Milit(api);
    }
    else if (this->playerID == 4)
    {
        Play_4_Milit(api);
    }
}

void AI::play(ITeamAPI& api)  // Ĭ��team playerID Ϊ0
{
    api.PrintSelfInfo();
    api.PrintTeam();
    Judge_4_Base(api);
    Base_Module_Install(api);
    Base_Build_Ship(api, 0);
}


void Base_Module_Install(ITeamAPI& api)
{
    auto ships = api.GetShips();
    auto selfinfo = api.GetSelfInfo();
    int id = selfinfo->teamID;
    int money = selfinfo->energy;

    Produce_Module(api, 1, 3);
    Produce_Module(api, 2, 3);

    if (ships.size() == 3)
    {
        Military_Module_weapon(api, 3, 4);
    }
    if (ships.size() == 4)
    {
        Construct_Module(api, 2, 3);
        Military_Module_shield(api, 4, 3);
        Military_Module_armour(api, 4, 3);
        Military_Module_weapon(api, 4, 4);
        Military_Module_armour(api, 3, 3);
        Military_Module_shield(api, 3, 3);
    }

    if (ships.size() == 4 && money > 100000)
    {
        Military_Module_weapon(api, 1, 1);
        Military_Module_weapon(api, 2, 1);
        Military_Module_shield(api, 1, 2);
        Military_Module_shield(api, 1, 2);
    }
    return;
}

bool GoCell(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    double speed = 3;
    if (selfinfo->shipType == THUAI7::ShipType::CivilianShip)
    {
        speed = SPEED_CIVIL_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::FlagShip)
    {
        speed = SPEED_FLAG_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::MilitaryShip)
    {
        speed = SPEED_MILIT_MS;
    }

    // ����һЩ����CELL������Ϣ
    std::string strx = std::to_string(cellx);
    std::string stry = std::to_string(celly);

    int Gcellx = 1000 * cellx + 500;
    int Gcelly = 1000 * celly + 500;

    if (abs(Gcellx - gridx) <= 30 && abs(Gcelly - gridy) <= 30)
    {
        return true;
    }
    else
    {
        if (Gcellx < gridx)
        {
            api.MoveUp((gridx - Gcellx) / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (Gcellx > gridx)
        {
            api.MoveDown((Gcellx - gridx) / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        if (Gcelly < gridy)
        {
            api.MoveLeft((gridy - Gcelly) / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (Gcelly > gridy)
        {
            api.MoveRight((Gcelly - gridy) / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        selfinfo = api.GetSelfInfo();
        gridx = selfinfo->x;
        gridy = selfinfo->y;
        cellx = api.GridToCell(gridx);
        celly = api.GridToCell(gridy);
        Gcellx = 1000 * cellx + 500;
        Gcelly = 1000 * celly + 500;

        if (abs(Gcellx - gridx) <= 30 && abs(Gcelly - gridy) <= 30)
        {
            api.Print("GoCell Finished !");
            return true;
        }
        else
        {
            api.Print("GoCell Did not Finish");
            return false;
        }
    }
}

void Base_Operate(ITeamAPI& api)
{
    auto selfships = api.GetShips();
    int number = selfships.size();
    if (number == 1)
    {
        Build_Ship(api, 2, 0);
    }
    else if (number == 2)
    {
        Build_Ship(api, 3, 0);
    }
    else if (number == 3)
    {
        Build_Ship(api, 4, 0);
    }
    return;
}

void Resource_Attack(IShipAPI& api)
{
    Get_Map(api);
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int size = resource_vec.size();
    if (size == 0)
    {
        api.Print("Please Get Resource ! ");
        return;
    }
    Update_Map(api);

Start:
    // minimum��ʾ·����Сֵ
    int minimum = 1000;
    // order��ʾ��С��Ӧ�ı��
    int order = -1;
    int x;
    int y;
    for (int i = 0; i < size; i++)
    {
        x = resource_vec[i].x;
        y = resource_vec[i].y;

        // Greedy�㷨�ҵ����Լ��������Դ,���ǵз���
        if (resource_vec[i].produce == false && ((selfinfo->teamID == 0 && x > 25) || (selfinfo->teamID == 1 && x < 23)))
        {
            auto path = findShortestPath(Map_grid, {cellx, celly}, {resource_vec[i].x_4p, resource_vec[i].y_4p}, api);
            int size = path.size();
            if (size < minimum && size > 0)
            {
                minimum = size;
                order = i;
            }
        }
    }
    if (order == -1)
    {
        // �з����Ҳ�����QaQ
        for (int i = 0; i < size; i++)
        {
            x = resource_vec[i].x;
            y = resource_vec[i].y;

            // Greedy�㷨�ҵ����Լ��������Դ,���Ǽ�����
            if (resource_vec[i].produce == false && ((selfinfo->teamID == 0 && x < 23) || (selfinfo->teamID == 1 && x > 25)))
            {
                auto path = findShortestPath(Map_grid, {cellx, celly}, {resource_vec[i].x_4p, resource_vec[i].y_4p}, api);
                int size = path.size();
                if (size < minimum && size > 0)
                {
                    minimum = size;
                    order = i;
                }
            }
        }
    }
    if (order == -1)
    {
        // ˫����û��
        // order==-1��ʾδ���ַ���Ҫ���
        api.Print("Finished!");
        return;
    }

    // ǰ������
    x = resource_vec[order].x;
    y = resource_vec[order].y;
    if (api.GetSelfInfo()->playerID % 2 == 1)
    {
        GoPlace_Loop(api, resource_vec[order].x_4p, resource_vec[order].y_4p);
    }
    else
    {
        GoPlace_Loop(api, resource_vec[order].x_4p2, resource_vec[order].y_4p2);
    }

    if (api.GetEnemyShips().size() > 0)
    {
        resource_vec[order].produce = true;
        AttackShip(api);
        goto Start;
    }
    else
    {
        resource_vec[order].produce = true;
        goto Start;
    }
    return;
}

void Install_Module(ITeamAPI& api, int number, int type)
{
    // ����number��ŵĴ�ֻ��װ��Ӧ��װ��
    auto selfships = api.GetShips();
    int size = selfships.size();
    if (type == 1)
    {
        // һ������װ��(Attack)
        api.InstallModule(number, THUAI7::ModuleType::ModuleMissileGun);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor2);
    }
    else if (type == 2)
    {
        // ��������װ��(Construct)
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        api.InstallModule(number, THUAI7::ModuleType::ModuleConstructor2);
    }
    else if (type == 3)
    {
        // ��������װ��(Comprehensive)
        api.InstallModule(number, THUAI7::ModuleType::ModuleArcGun);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        api.InstallModule(number, THUAI7::ModuleType::ModuleConstructor1);
    }
    return;
}

void AttackShip(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int intenddis = 4000;
    auto weapon = selfinfo->weaponType;
    double speed = 3.0;
    if (selfinfo->shipType == THUAI7::ShipType::CivilianShip)
    {
        speed = SPEED_CIVIL_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::FlagShip)
    {
        speed = SPEED_FLAG_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::MilitaryShip)
    {
        speed = SPEED_MILIT_MS;
    }
    if (weapon == THUAI7::WeaponType::LaserGun || weapon == THUAI7::WeaponType::PlasmaGun || weapon == THUAI7::WeaponType::ShellGun)
    {
        intenddis = 4000;
    }
    else if (weapon == THUAI7::WeaponType::NullWeaponType)
    {
        intenddis = 0;
        // û��������
        // hide(api);
        return;
    }
    else
    {
        intenddis = 6000;
    }

Start:
    if (!Update_Enemy(api))
    {
        return;
    }
    int number = Enemy_Attack_Index(api);
    int hp = enemy_vec[number].hp;
    double angle = Count_Angle(api, enemy_vec[number].gridx, enemy_vec[number].gridy);
    int distance = (gridx - enemy_vec[number].gridx) * (gridx - enemy_vec[number].gridx) + (gridy - enemy_vec[number].gridy) * (gridy - enemy_vec[number].gridy);
    if (sqrt(distance) >= intenddis)
    {
        // ���������
        // ������÷����ƶ��Ļ��ƣ����С�׷����
        api.Move((distance - intenddis + 100) / (2 * speed), angle);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        api.Move((distance - intenddis + 100) / (2 * speed), angle);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (!Update_Enemy(api))
        {
            return;
        }
        number = Enemy_Attack_Index(api);
        hp = enemy_vec[number].hp;
        angle = Count_Angle(api, enemy_vec[number].gridx, enemy_vec[number].gridy);
        distance = (gridx - enemy_vec[number].gridx) * (gridx - enemy_vec[number].gridx) + (gridy - enemy_vec[number].gridy) * (gridy - enemy_vec[number].gridy);
    }
    int count = 0;
    int round = 0;
    // ������ڣ����й���
    while (round < 30)
    {
        api.Attack(angle);
        count++;

        if (count % 5 == 0)
        {
            round++;
            count = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            if (!Update_Enemy(api))
            {
                return;
            }
            else
            {
                number = Enemy_Attack_Index(api);
                hp = enemy_vec[number].hp;
                angle = Count_Angle(api, enemy_vec[number].gridx, enemy_vec[number].gridy);
            }
            if (hp == 0)
            {
                kill_number++;
                // ���ﲻӦѡ��continue����Ϊcontinue�ᵼ��i��������
                // ���п��ܲ���Խ��
                // continue;
                goto Start;
            }
        }
    }
    return;
}

void hide(IShipAPI& api)
{
    int gridx = api.GetSelfInfo()->x;
    int gridy = api.GetSelfInfo()->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);
    auto map = api.GetFullMap();
    int HP = api.GetSelfInfo()->hp;
    
    auto enemyships = api.GetEnemyShips();
    int size = enemyships.size();
    int enemymosthp = 0;
    bool enemyflag = false;
    int first_en = -1;
    for (int i = 0; i < size; i++)
        if (enemyships[i]->hp > 0 && enemyships[i]->weaponType != THUAI7::WeaponType::NullWeaponType && enemyships[i]->hp > enemymosthp)
        {
            enemyflag = true;
            enemymosthp = enemyships[i]->hp;
            first_en = i;
        }
    // ������˿Ѫ���Σ���shadow��
    for (int i = cellx - 10 < 0 ? 0 : cellx - 10; i < (cellx + 11 > 50 ? 50 : cellx + 11)&& map[cellx][celly]!=THUAI7::PlaceType::Shadow ; i++)  // ��Ҫ���������+����Ұ���ж�
    {
        for (int j = celly - sqrt(10 * 10 - (i - cellx) * (i - cellx)) < 0 ? 0 : celly - sqrt(10 * 10 - (i - cellx) * (i - cellx)); j < (celly + sqrt(10 * 10 - (i - cellx) * (i - cellx)) + 1 > 50 ? 50 : celly + sqrt(10 * 10 - (i - cellx) * (i - cellx)) + 1); j++)
        {
            if (map[i][j] == THUAI7::PlaceType::Shadow)
            {
                GoPlace_Loop(api, i, j);
                api.Wait();
                enemyships = api.GetEnemyShips();
                gridx = api.GetSelfInfo()->x;
                gridy = api.GetSelfInfo()->y;
                cellx = api.GridToCell(gridx);
                celly = api.GridToCell(gridy);
                size = enemyships.size();
                enemyflag = false;
                for (int i = 0; i < size; i++)
                    if (enemyships[i]->hp > 0)
                    {
                        enemyflag = true;
                        break;
                    }
                if (enemyflag && map[cellx][celly] != THUAI7::PlaceType::Shadow)  // ����underattack�������ж�
                    return hide(api);
                else
                    return;
            }
        }
    }
    if(enemyflag) //Ŀǰֻ��Զ���˵�һ�����ˣ���ȻҪSVM�ˣ���Ƚϳ���
    {
        auto Enemy1x = enemyships[first_en]->x;
        auto Enemy1y = enemyships[first_en]->y;
        int dis1x = Enemy1x - gridx;
        int dis1y = Enemy1y - gridy;
        double angle1 = atan(dis1y / dis1x);
        double distance1 = sqrt(dis1x * dis1x + dis1y * dis1y);
        GoPlace_Loop(api,cellx+(int)((8-distance1) * cos(angle1+pi)), celly+(int)((8-distance1) * sin(angle1+pi)));
    }
        // �����Ǹ�֪baseҪװװ����Ҫ����ȫ�ֱ���������Ϣ��base�ڵĺ��������ٽ����жϺ;��ߣ���Ϣ����Ҫ���߼����ж�
        /* modl1.number = api.GetSelfInfo()->playerID;
        modl1.moduletype = THUAI7::ModuleType::ModuleShield3;*/
}

bool attack(IShipAPI& api)  // ���ڸģ�ȱ�ݻ��ܶ�
{
    int gridx = api.GetSelfInfo()->x;
    int gridy = api.GetSelfInfo()->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);
    auto map = api.GetFullMap();
    int HP = api.GetSelfInfo()->hp;

    auto enemyships = api.GetEnemyShips();
    int totalenenmyhp = 0;
    int size = enemyships.size();

    for (int i = 0; i<size; i++)
        totalenenmyhp += enemyships[i]->hp;

    if (size)       //Ҫ����shadow���ܹ���������
    {
        if (HP < 1500 && totalenenmyhp > HP && map[cellx][celly]!=THUAI7::PlaceType::Shadow)  // �������ܻ�Ҫ�ģ�˿Ѫ����
        {
            hide(api);
            api.Wait();
        }
        // ����
        if (api.GetSelfInfo()->weaponType != THUAI7::WeaponType::NullWeaponType)
        {
            AttackShip(api);
        }
        else
        {
            hide(api);  // ��ʱ���ţ����������뺯�����������������˿Ѫ���κϲ�����
        }
        //Send_Me(api, "003");
        //Send_Me(api, "013");
        return false;
    }
    else
        return true;
}

std::vector<std::vector<int>> Get_Map(IShipAPI& api)
{
    auto map = api.GetFullMap();
    auto selfinfo = api.GetSelfInfo();
    int cellx = api.GridToCell(selfinfo->x);
    int count_re = -1;
    int TeamID = selfinfo->teamID;

    if (TeamID == 0)
    {
        home_vec.push_back(my_Home(3, 46, 1));
        home_vec[0].x_4p = 3;
        home_vec[0].y_4p = 47;
        home_vec[0].HP = api.GetGameInfo()->redHomeHp;
        home_vec.push_back(my_Home(46, 3, 2));
        home_vec[1].x_4p = 48;
        home_vec[1].y_4p = 3;
        home_vec[1].HP = api.GetGameInfo()->blueHomeHp;
    }
    else if (TeamID == 1)
    {
        home_vec.push_back(my_Home(46, 3, 1));
        home_vec[0].x_4p = 46;
        home_vec[0].y_4p = 4;
        home_vec[0].HP = api.GetGameInfo()->blueHomeHp;
        home_vec.push_back(my_Home(3, 46, 2));
        home_vec[1].x_4p = 5;
        home_vec[1].y_4p = 46;
        home_vec[1].HP = api.GetGameInfo()->redHomeHp;
    }

    for (int i = 0; i < map_size; i++)
    {
        for (int j = 0; j < map_size; j++)
        {
            if (map[i][j] == THUAI7::PlaceType::Space || map[i][j] == THUAI7::PlaceType::Shadow || map[i][j] == THUAI7::PlaceType::Wormhole)
            {  // �ɾ����ĵص�Ϊ0��Ĭ��(������;��)Ϊ1 �Ѿ�Ϊ 2
                Map_grid[i][j] = 0;
                if (map[i][j] == THUAI7::PlaceType::Space || map[i][j] == THUAI7::PlaceType::Shadow)
                {
                    continue;
                }
                auto hp = api.GetWormholeHp(i, j);
                if (j == 24 || j == 25)
                {
                    if (hp == -1)
                    {
                        wormhole_vec.push_back(my_Wormhole(i, j, 18000));
                        hp = 24000;
                    }
                    else
                    {
                        wormhole_vec.push_back(my_Wormhole(i, j, hp));
                        if (hp <= 12000)
                        {
                            Map_grid[i][j] = 1;
                        }
                    }
                }
                else
                {
                    wormhole_vec.push_back(my_Wormhole(i, j, hp));
                    if (hp <= 12000)
                    {
                        Map_grid[i][j] = 1;
                    }
                }
            }
            else if (map[i][j] == THUAI7::PlaceType::Resource)
            {
                count_re++;
                if (map[i + 1][j] == THUAI7::PlaceType::Space || map[i + 1][j] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i + 1, j));
                    if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i - 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j + 1;
                        continue;
                    }
                    else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j - 1;
                        continue;
                    }
                    continue;
                }
                else if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i - 1, j));
                    if (map[i + 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i + 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j + 1;
                        continue;
                    }
                    else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j - 1;
                        continue;
                    }
                    continue;
                }
                else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i, j + 1));
                    if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i - 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i + 1][j] == THUAI7::PlaceType::Space || map[i + 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i + 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j - 1;
                        continue;
                    }
                    continue;
                }
                else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i, j - 1));
                    if (map[i + 1][j] == THUAI7::PlaceType::Space || map[i + 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i + 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i - 1;
                        resource_vec[count_re].y_4p2 = j;
                        continue;
                    }
                    else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                    {
                        resource_vec[count_re].x_4p2 = i;
                        resource_vec[count_re].y_4p2 = j + 1;
                        continue;
                    }
                    continue;
                }
            }
            else if (map[i][j] == THUAI7::PlaceType::Construction)
            {
                if (map[i + 1][j] == THUAI7::PlaceType::Space || map[i + 1][j] == THUAI7::PlaceType::Shadow)
                {
                    construction_vec.push_back(my_Construction(i, j, i + 1, j));
                    continue;
                }
                else if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                {
                    construction_vec.push_back(my_Construction(i, j, i - 1, j));
                    continue;
                }
                else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                {
                    construction_vec.push_back(my_Construction(i, j, i, j + 1));
                    continue;
                }
                else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                {
                    construction_vec.push_back(my_Construction(i, j, i, j - 1));
                    continue;
                }
            }
        }
    }

    int size = construction_vec.size();
    int temp = 1000;
    int number = -1;
    for (int i = 0; i < size; i++)
    {
        if (construction_vec[i].home_dist < temp)
        {
            temp = construction_vec[i].home_dist;
            number = i;
        }
    }
    if (construction_vec[number].home_dist < 64)
    {
        closest_2_home = construction_vec[number];
        index_close = number;
        api.Print("Get Consturction Point Closest to Home !\n Can Build Fort In This Place");
    }
    api.Print("The Map Has Already Got !");
    return Map_grid;
}

bool GoPlace(IShipAPI& api, int des_x, int des_y)
{
    if (des_x < 0)
        return GoPlace(api, 0, des_y);
    if (des_x >= 50)
        return GoPlace(api, 49, des_y);
    if (des_y < 0)
        return GoPlace(api, des_x, 0);
    if (des_y >= 50)
        return GoPlace(api, des_x, 49);
    if (Map_grid[des_x][des_y] == 1)
    {
        if (des_x-1>=0&&Map_grid[des_x - 1][des_y] == 0)
            return GoPlace(api, des_x - 1, des_y);
        else if (des_x+1<50&&Map_grid[des_x + 1][des_y] == 0)
            return GoPlace(api, des_x + 1, des_y);
        else if (des_y-1>=0&&Map_grid[des_x][des_y - 1] == 0)
            return GoPlace(api, des_x, des_y - 1);
        else if (des_y+1<50&&Map_grid[des_x][des_y + 1] == 0)
            return GoPlace(api, des_x, des_y + 1);
        else
            return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto selfinfo = api.GetSelfInfo();
    int cur_gridx = selfinfo->x;
    int cur_gridy = selfinfo->y;
    int cur_x = api.GridToCell(cur_gridx);
    int cur_y = api.GridToCell(cur_gridy);
    double speed = 3.0;
    if (selfinfo->shipType == THUAI7::ShipType::CivilianShip)
    {
        speed = SPEED_CIVIL_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::FlagShip)
    {
        speed = SPEED_FLAG_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::MilitaryShip)
    {
        speed = SPEED_MILIT_MS;
    }

    if (cur_x == des_x && cur_y == des_y)
    {
        api.Print("Have Already Reached the Place! ");
        return true;
    }
    Point start = {cur_x, cur_y};
    Point end = {des_x, des_y};

    std::vector<Point> path = findShortestPath(Map_grid, start, end, api);
    int path_size = path.size();
    // ע�͵��������ж�Ѱ·�㷨�ȶ��ԵĴ���
    // std::string str = std::to_string(path_size);
    // api.Print("This is the Size of the Path");
    // api.Print(str);
    for (int i = 0; i < path_size - 1; i++)
    {
        direction[i] = Point{
            path[i + 1].x - path[i].x,
            path[i + 1].y - path[i].y};
    }
    // api.Print("Got the Direction of the PATH !");
    // api.Wait();
    for (int j = 0; j < path_size - 1; j++)
    {
        if (j % 10 == 0 && j > 0)
        {  // ÿ�ƶ���ν���һ��GoCell
            GoCell(api);
            Judge_4_Civil(api);
            if (api.GridToCell(api.GetSelfInfo()->x) == cur_x && api.GridToCell(api.GetSelfInfo()->y) == cur_y)
            {  // û�б仯����ס�ˣ�����������
                goto Restart;
            }
            if (Update_Enemy(api))
                once_met_enemys = true;
        }
        if (direction[j].x == 1 && direction[j].y == 1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == 1 && direction[j].y == -1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1 && direction[j].y == 1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1 && direction[j].y == -1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1)
        {
            AttackShip(api);
            api.MoveUp(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (direction[j].x == 1)
        {
            AttackShip(api);
            api.MoveDown(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (direction[j].y == -1)
        {
            AttackShip(api);
            api.MoveLeft(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else
        {
            AttackShip(api);
            api.MoveRight(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
    }
    GoCell(api);
    selfinfo = api.GetSelfInfo();
    cur_gridx = selfinfo->x;
    cur_gridy = selfinfo->y;
    cur_x = api.GridToCell(cur_gridx);
    cur_y = api.GridToCell(cur_gridy);
    if (cur_x == des_x && cur_y == des_y)
    {
        return true;
    }
    return false;
Restart:
    return GoPlace(api, des_x, des_y);
}

bool GoPlace_Loop(IShipAPI& api, int des_x, int des_y)
{
    bool temp = false;
    int count = 0;
    while (temp == false && count < 7)
    {
        temp = GoPlace(api, des_x, des_y);
        count++;
    }
    if (temp == true)
    {
        api.Print("Arrived! Loop Terminated! ");
        return true;
    }
    else
    {
        api.Print("Not Arrived Yet! Loop Terminated! ");
        return false;
    }
}

bool isValid(IShipAPI& api, int x, int y)
{
    return (x >= 0 && x < map_size  && y >= 0 && y < map_size );
}

const std::vector<Point> findShortestPath(const std::vector<std::vector<int>>& grid, Point start, Point end, IShipAPI& api)
{
    // �ȸ��µ�ͼ
    Update_Map(api);
    // ��¼ÿ�����Ƿ񱻷��ʹ�
    std::vector<std::vector<bool>> visited(map_size, std::vector<bool>(map_size, false));
    // ��¼ÿ�����ǰ���ڵ㣬������������·��
    std::vector<std::vector<Point>> parent(map_size, std::vector<Point>(map_size, {-1, -1}));
    // ����һ����������BFS
    std::queue<Point> q;

    // �������Ϊ�ѷ��ʣ����������
    visited[start.x][start.y] = true;
    q.push(start);

    // ����һ��·��vector
    std::vector<Point> Path;

     int count = 0;
    // BFS
    while (!q.empty())
    {
        Point current = q.front();
        q.pop();

        // �����ǰ�����յ㣬˵�����ҵ�·����ֹͣ����
        if ((current.x == end.x) && (current.y == end.y))
        {
            break;
        }
        // ������ǰ����ĸ�����
        for (int i = 0; i < 8; ++i)
        {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];

            if (dx[i] * dy[i])
            {  // ��������б���н��������Ҫ�ж�����������
                if (grid[newX][current.y] || grid[current.x][newY])
                {  // ���������ϰ���������
                    continue;
                }
            }
            // ����������ڵ�ͼ��Χ����δ���ʹ��Ҳ����ϰ����������
            if (isValid(api, newX, newY) && !visited[newX][newY] && (grid[newX][newY] == 0))
            {
                visited[newX][newY] = true;
                parent[newX][newY] = {current.x, current.y};
                q.push({newX, newY});
            }
        }
    }

    // ����·��
    if (visited[end.x][end.y])
    {
        Point current = end;
        while (current.x != start.x || current.y != start.y)
        {
            Path.push_back(current);
            current = parent[current.x][current.y];
        }
        Path.push_back(start);
        reverse(Path.begin(), Path.end());
        api.Print("The Path Have Already Got!");
    }
    else
    {
        api.Print("No path found! ");
        std::vector<Point> temp;
        return temp;
    }
    return Path;
}

void Get_Resource(IShipAPI& api)
{  // ����ȫ�Զ���Դ���ɣ�ֱ���ѱ�����ͼ��������Դ������Ϊֹ
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int size = resource_vec.size();
    if (size == 0)
    {
        api.Print("Please Get Resource ! ");
        return;
    }
    int distance;
    int temp;
    int order;

    for (int i = 0; i < size; i++)
    {
        int x = resource_vec[i].x;
        int y = resource_vec[i].y;

        if ((cellx <= 25 && x <= 25) || (cellx >= 27 && x >= 27))
        {
            GoPlace_Loop(api, resource_vec[i].x_4p, resource_vec[i].y_4p);
            int state = api.GetResourceState(x, y);
            api.Print(std::to_string(state));
            int count = 0;
            while (state > 0)
            {  // ֻҪ����ʣ����Դ�Ϳ���
                api.Produce();
                count++;
                if (count % 10 == 0)
                {
                    // ÿʮ�ν���һ���ж��뷵��
                    api.Wait();
                    state = api.GetResourceState(x, y);
                    resource_vec[i].HP = state;
                    count = 0;
                }
            }
        }
    }
}

void Greedy_Resource(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int size = resource_vec.size();
    if (size == 0)
    {
        api.Print("Please Get Resource ! ");
        return;
    }
    // minimum��ʾ·����Сֵ
    int minimum = 1000;
    // order��ʾ��С��Ӧ�ı��
    int order = -1;
    int x;
    int y;

    Update_Map(api);

    for (int i = 0; i < size; i++)
    {
        x = resource_vec[i].x;
        y = resource_vec[i].y;

        // Greedy�㷨�ҵ����Լ��������Դ
        if (resource_vec[i].produce == false)
        {
            auto path = findShortestPath(Map_grid, {cellx, celly}, {resource_vec[i].x_4p, resource_vec[i].y_4p}, api);
            int size = path.size();
            if (size < minimum && size > 0)
            {
                minimum = size;
                order = i;
            }
        }
    }
    if (order == -1)
    {
        // order==-1��ʾδ���ַ���Ҫ���
        api.Print("Finished!");
        return;
    }

    // ǰ������
    x = resource_vec[order].x;
    y = resource_vec[order].y;
    if (api.GetSelfInfo()->playerID % 2 == 1)
    {
        GoPlace_Loop(api, resource_vec[order].x_4p, resource_vec[order].y_4p);
    }
    else
    {
        GoPlace_Loop(api, resource_vec[order].x_4p2, resource_vec[order].y_4p2);
    }
    int state = api.GetResourceState(x, y);

    int count = 0;
    while (state > 0)
    {  // ֻҪ����ʣ����Դ�Ϳ���
        api.Produce();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        count++;
        if (count % 10 == 0)
        {
            Judge_4_Civil(api);
            // ÿʮ�ν���һ���ж��뷵��
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            state = api.GetResourceState(x, y);
            resource_vec[order].HP = state;
            count = 0;
        }
    }

    if (state == 0)
    {
        resource_vec[order].produce = true;
        Greedy_Resource(api);
    }
    else
    {
        api.Print("Abrupted!");
        return;
    }
}

void Build_ALL(IShipAPI& api, THUAI7::ConstructionType type)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int IntendedHp = 10000;
    int size = construction_vec.size();
    if (size == 0)
    {
        api.Print("NO VALID CONSTRUCTION POINT!");
        return;
    }
    if (type == THUAI7::ConstructionType::Factory)
    {
        IntendedHp = 12000;
    }
    else if (type == THUAI7::ConstructionType::Fort)
    {
        IntendedHp = 16000;
    }
    else if (type == THUAI7::ConstructionType::Community)
    {
        IntendedHp = 10000;
    }
    for (int j = 0; j < 10; j++)
    {
        // ���ڲ�����Construct����������Energy����Energy�����ᵼ����ֹ
        // ������ǲ���ʮ��ѭ����ʹ�þ����ܶཨ��
        for (int i = 0; i < size; i++)
        {
            if (((cellx <= 25 && construction_vec[i].x <= 25) || (cellx >= 27 && construction_vec[i].x >= 27)) && construction_vec[i].HP < IntendedHp && construction_vec[i].build == false)
            {
                GoPlace_Loop(api, construction_vec[i].x_4c, construction_vec[i].y_4c);
                int Hp = 0;
                if (api.GetConstructionState(construction_vec[i].x, construction_vec[i].y).has_value())
                {
                    Hp = api.GetConstructionState(construction_vec[i].x, construction_vec[i].y)->hp;
                }
                int round = 0;
                if (Hp < IntendedHp && round < 85)
                {
                    int count = 0;
                    while (Hp < IntendedHp)
                    {
                        api.Construct(type);
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        count++;
                        if (count % 10 == 0)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            if (api.GetConstructionState(construction_vec[i].x, construction_vec[i].y).has_value())
                            {
                                Hp = api.GetConstructionState(construction_vec[i].x, construction_vec[i].y)->hp;
                            }
                            count = 0;
                            round++;
                            construction_vec[i].HP = Hp;
                        }
                    }
                }
                if (Hp >= IntendedHp / 2 || round > 80)
                {
                    construction_vec[i].build = true;
                }
            }
        }
    }
}

void Construct_Module(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0)
    {
        api.Print("No Correspond Ship!  \n");
        return;
    }

    auto constructtype = ships[shipno - 1]->constructorType;

    int energy = api.GetEnergy();
    if (energy < 4000 || (ships[shipno - 1]->shipType != THUAI7::ShipType::CivilianShip && ships[shipno - 1]->shipType != THUAI7::ShipType::FlagShip))
    {
        return;
    }
    switch (type)
    {
        case 2:
            if (energy > 4000)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleConstructor2);
            }
            break;

        case 3:
            if (energy > 8000)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleConstructor3);
            }

            break;
    }
    return;
}

void Produce_Module(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0)
    {
        return;
    }
    if (ships[shipno - 1] == nullptr || (ships[shipno - 1]->shipType != THUAI7::ShipType::CivilianShip && ships[shipno - 1]->shipType != THUAI7::ShipType::FlagShip))
    {
        return;
    }
    auto producetype = ships[shipno - 1]->producerType;
    int energy = api.GetEnergy();
    switch (type)
    {
        case 2:
            if (producetype == THUAI7::ProducerType::Producer1 && energy > 4000)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleProducer2);
            }
        case 3:
            if (producetype != THUAI7::ProducerType::Producer3 && energy > 8000)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleProducer3);
            }
    }
    return;
}

void Build_Ship(ITeamAPI& api, int shipno, int birthdes)
{
    api.Wait();
    auto selfinfo = api.GetSelfInfo();
    int energy = selfinfo->energy;
    auto ships = api.GetShips();
    int size = ships.size();
    int milit_count = 0;
    int civil_count = 0;

    for (int i = 0; i < size; i++)
    {
        if (ships[i]->shipType == THUAI7::ShipType::CivilianShip && ships[i]->hp > 1000)
            civil_count++;
        else if ((ships[i]->shipType == THUAI7::ShipType::MilitaryShip || ships[i]->shipType == THUAI7::ShipType::FlagShip) && ships[i]->hp > 1000)
            milit_count++;
    }

    switch (shipno)
    {
        case 1:
            // ��û����Ҫ����
            if (civil_count == 2)
                return;
        case 2:
            // ͬ��
            if (civil_count == 2)
                return;
        case 3:
            if (milit_count == 2)
                return;
        case 4:
            if (milit_count == 2)
                return;
    }
    if (ShipTypeDict[shipno - 1] == THUAI7::ShipType::CivilianShip)
    {
        if (energy > 4000)
        {
            api.BuildShip(THUAI7::ShipType::CivilianShip, birthdes);
        }
        else
        {
            api.Print("No Enough Money!");
        }
        return;
    }
    else if (ShipTypeDict[shipno - 1] == THUAI7::ShipType::MilitaryShip)
    {
        if (energy > 12000)
        {
            api.BuildShip(THUAI7::ShipType::MilitaryShip, birthdes);
        }
        else
        {
            api.Print("No Enough Money!");
        }
    }
    else if (ShipTypeDict[shipno - 1] == THUAI7::ShipType::FlagShip)
    {
        if (energy > 50000)
        {
            api.BuildShip(THUAI7::ShipType::FlagShip, birthdes);
        }
        else
        {
            api.Print("No Enough Money!");
        }
    }
    return;
}

void Build_Specific(IShipAPI& api, THUAI7::ConstructionType type, int index)
{
    auto selfinfo = api.GetSelfInfo();
    int teamid = selfinfo->teamID;
    // ��ü�����Baseû���˵�����½���Community
    // �������������Ѿ�����
    if (index == -1)
    {  // û�з���Ҫ��ĵ�
        return;
    }
    // ���� ֱ�Ӷ��ڶ�������޸�
    my_Construction& construction = construction_vec[index];
    int hp = construction.HP;
    if (construction.type == THUAI7::ConstructionType::Community || construction.build == true)
    {
        if (hp >= 10000)
        {
            return;
        }
    }
    else if (construction.type == THUAI7::ConstructionType::Factory || construction.build == true)
    {
        if (hp >= 8000)
        {
            return;
        }
    }
    else if (construction.type == THUAI7::ConstructionType::Fort || construction.build == true)
    {
        if (hp >= 16000)
        {
            return;
        }
    }

    // �������е��˴�
    GoPlace_Loop(api, construction.x_4c, construction.y_4c);
    bool temp;
    int count = 0;
    int round = 0;
    int IntendedHp = 6000;
    if (api.GetConstructionState(construction.x, construction.y).has_value())
    {
        hp = api.GetConstructionState(construction.x, construction.y)->hp;
    }
    if (type == THUAI7::ConstructionType::Factory)
    {
        IntendedHp = 12000;
    }
    else if (type == THUAI7::ConstructionType::Fort)
    {
        IntendedHp = 16000;
    }
    else if (type == THUAI7::ConstructionType::Community)
    {
        IntendedHp = 10000;
    }
    while (hp < IntendedHp && round < 150)
    {
        count++;
        api.Construct(type);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (count % 10 == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (api.GetConstructionState(construction.x, construction.y).has_value())
            {
                hp = api.GetConstructionState(construction.x, construction.y)->hp;
            }
            count = 0;
            round++;
        }
    }
    if (hp == IntendedHp || round > 100)
    {
        construction.HP = hp;
        construction.build = true;
        construction.type = type;
        api.Print("Construct Successfully Finished !");
    }
    return;
}

void Decode_Me_4_Milit(IShipAPI& api)
{
    // ��������message
    if (api.HaveMessage())
    {
        auto message = api.GetMessage();
        api.Print("Get Message!");
        int from = message.first;
        if (from == 0)
        {
            // �ط�����
            auto info = message.second;
            auto info1 = info.c_str();
            int place_x = 10 * (info1[1] - '0') + (info[2] - '0');
            int place_y = 10 * (info1[3] - '0') + (info[4] - '0');
            GoPlace_Loop(api, place_x, place_y);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            int round = 0;
            while (round < 20)
            {
                AttackShip(api);
                round++;
            }
        }
        else
        {
            auto info = message.second;
            if (info[0] == '1')
            {
                auto info1 = info.c_str();
                int place_x = 10 * (info1[1] - '0') + (info[2] - '0');
                int place_y = 10 * (info1[3] - '0') + (info[4] - '0');
                // �Ѿ����ѣ�������ɽ��bushi��
                GoPlace_Loop(api, place_x, place_y);
                AttackShip(api);
            }
        }
    }
    api.Wait();
    return;
}

void Send_Me(IShipAPI& api, std::string str)
{
    // �ǳ���ª����Ҫ�ǵ��׷���ʲô��Ϣ��Ҫ��Attack/Hide������òź���
    if (str.c_str()[0] == '0')
    {
        api.SendBinaryMessage(0, str);
    }
    else
    {
        api.SendBinaryMessage(str.c_str()[0] - '0', str);
    }
    return;
}

void Send_Me(ITeamAPI& api)
{
    int hp = api.GetHomeHp();
    api.Wait();
    int hp1 = api.GetHomeHp();
    if (hp1 < hp)
    {
        api.SendBinaryMessage(1, "0");
        api.SendBinaryMessage(2, "0");
        api.SendBinaryMessage(3, "0");
        api.SendBinaryMessage(4, "0");
    }
    return;
}

void Decode_Me(ITeamAPI& api)
{
    // ���ؽ���
    if (api.HaveMessage())
    {
        auto message = api.GetMessage();
        int number = message.first;
        auto info = message.second.c_str();
        if (info[0] == '0')
        {
            Military_Module_shield(api, number, info[1] - '0');
        }
        else if (info[0] == '1')
        {
            Military_Module_armour(api, number, info[1] - '0');
        }
        else if (info[0] == '2')
        {
            Military_Module_weapon(api, number, info[1] - '0');
        }
    }
}

void Greedy_Build(IShipAPI& api, THUAI7::ConstructionType type)
{

    // ̰���㷨����·�����н���
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int IntendedHp = 10000;
    if (type == THUAI7::ConstructionType::Factory)
    {
        IntendedHp = 12000;
    }
    else if (type == THUAI7::ConstructionType::Fort)
    {
        IntendedHp = 16000;
    }
    else if (type == THUAI7::ConstructionType::Community)
    {
        IntendedHp = 10000;
    }

    int size = construction_vec.size();
    if (size == 0)
    {
        api.Print("Please Get Construction ! ");
        return;
    }
Start:
    // minimum��ʾ·����Сֵ
    int minimum = 1000;
    // order��ʾ��С��Ӧ�ı��
    int order = -1;
    int x;
    int y;
    Update_Map(api);
    for (int i = 0; i < size; i++)
    {
        x = construction_vec[i].x;
        y = construction_vec[i].y;

        // Greedy�㷨�ҵ����Լ��������Դ
        if (construction_vec[i].build == false)
        {
            auto path = findShortestPath(Map_grid, {cellx, celly}, {construction_vec[i].x_4c, construction_vec[i].y_4c}, api);
            int size = path.size();
            if (size < minimum && size > 0)
            {
                minimum = size;
                order = i;
            }
        }
    }
    if (order == -1)
    {
        // order==-1��ʾδ���ַ���Ҫ���
        api.Print("Finished!");
        return;
    }

    // ǰ������
    x = construction_vec[order].x;
    y = construction_vec[order].y;
    GoPlace_Loop(api, construction_vec[order].x_4c, construction_vec[order].y_4c);
    int hp = 0;
    if (api.GetConstructionState(x, y).has_value())
    {
        auto state = api.GetConstructionState(x, y);
        hp = state->hp;
        if (state->teamID != selfinfo->teamID)
        {
            construction_vec[order].group = 2;
            construction_vec[order].build = true;
        }
    }
    int round = 0;
    int count = 0;
    while (hp < IntendedHp && round < 150)
    {  // û�ﵽԤ��Hp �ͼ�������
        api.Construct(type);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        count++;
        if (count % 10 == 0)
        {
            // ÿʮ�ν���һ���ж��뷵��
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (api.GetConstructionState(x, y).has_value())
            {
                hp = api.GetConstructionState(x, y)->hp;
            }
            construction_vec[order].HP = hp;
            count = 0;
            round++;
        }
    }
    Judge_4_Civil(api);
    if (hp >= IntendedHp / 2 || round > 100)
    {  // ����ﵽ��Ԥ�ڽ�����Ѫ����һ�룬�ͱ��Ϊ�Ѿ��������
        // ���Խ�����������ͻȻ�Ͽ���
        // ����ʵ����get hpò����bug���������Ǽ��뽨������round��Ϊ�жϱ�׼
        construction_vec[order].build = true;
        construction_vec[order].group = 1;
        Greedy_Build(api, type);
    }
    else
    {
        api.Print("Abrupted!");
        return;
    }
}

void Update_Map(IShipAPI& api)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto friendinfo = api.GetShips();
    int size = friendinfo.size();
    int selfnumber = api.GetSelfInfo()->playerID;
    auto map = api.GetFullMap();
    int worm_index = 0;

    if (size <= 0)
    {
        return;
    }
    else
    {
        for (int x = 0; x < map_size; x++)
        {
            for (int y = 0; y < map_size; y++)
            {
                if (Map_grid[x][y] == 2)
                {
                    // ֮ǰ�жϵ��Ѿ�λ������
                    Map_grid[x][y] = 0;
                }
                if (map[x][y] == THUAI7::PlaceType::Wormhole)
                {
                    int hp = api.GetWormholeHp(x, y);
                    wormhole_vec[worm_index].HP = hp;
                    if (hp < 9000 && hp >= 0)
                    {
                        // ����õ���hp���ڵ���0С��9000
                        // ���涴�ر�
                        Map_grid[x][y] = 1;
                    }
                }
            }
        }
        for (int i = 0; i < size; i++)
        {
            if (friendinfo[i]->playerID == selfnumber)
            {
                continue;
            }
            else
            {
                // �����Ѿ�λ��
                int tempx = api.GridToCell(friendinfo[i]->x);
                int tempy = api.GridToCell(friendinfo[i]->y);
                Map_grid[tempx][tempy] = 2;
            }
        }
        api.Print("Map Update Finished!");
    }
    int re_size = resource_vec.size();
    std::vector<my_Resource>& temp = resource_vec;
    for (int i = 0; i < re_size; i++)
    {
        int tempx = temp[i].x;
        int tempy = temp[i].y;
        if (Map_grid[tempx + 1][tempy] == 0)
        {
            temp[i].x_4p = tempx + 1;
            temp[i].y_4p = tempy;
        }
        else if (Map_grid[tempx - 1][tempy] == 0)
        {
            temp[i].x_4p = tempx - 1;
            temp[i].y_4p = tempy;
        }
        else if (Map_grid[tempx][tempy + 1] == 0)
        {
            temp[i].x_4p = tempx;
            temp[i].y_4p = tempy + 1;
        }
        else if (Map_grid[tempx][tempy - 1] == 0)
        {
            temp[i].x_4p = tempx;
            temp[i].y_4p = tempy - 1;
        }
    }

    std::vector<my_Construction>& temp2 = construction_vec;
    int con_size = temp2.size();
    for (int i = 0; i < con_size; i++)
    {
        int tempx = temp2[i].x;
        int tempy = temp2[i].y;
        if (Map_grid[tempx + 1][tempy] == 0)
        {
            temp2[i].x_4c = tempx + 1;
            temp2[i].y_4c = tempy;
        }
        else if (Map_grid[tempx - 1][tempy] == 0)
        {
            temp2[i].x_4c = tempx - 1;
            temp2[i].y_4c = tempy;
        }
        else if (Map_grid[tempx][tempy + 1] == 0)
        {
            temp2[i].x_4c = tempx;
            temp2[i].y_4c = tempy + 1;
        }
        else if (Map_grid[tempx][tempy - 1] == 0)
        {
            temp2[i].x_4c = tempx;
            temp2[i].y_4c = tempy - 1;
        }
    }
    api.Print("Point Available Updated!");
    return;
}

void Military_Module(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0)
    {
        return;
    }
    Decode_Me(api);
    
}

void Military_Module_weapon(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0) //��û������
    {
        return;
    }
    auto weapontype = ships[shipno - 1]->weaponType;
    int energy = api.GetEnergy();
    if (ShipTypeDict[shipno - 1] == THUAI7::ShipType::CivilianShip)
    {
        if (ships[shipno - 1]->weaponType == THUAI7::WeaponType::NullWeaponType && energy >= 10000)
        {
            api.InstallModule(shipno, THUAI7::ModuleType::ModuleLaserGun);
        }
        return;
    }
    if (ShipTypeDict[shipno - 1] == THUAI7::ShipType::MilitaryShip || ShipTypeDict[shipno - 1] == THUAI7::ShipType::FlagShip)
    {
        switch (type)   //���绡�ڲ���Ϊ�߼���ʣ�µĶ����˼�������
        {
            case 1:
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleLaserGun);  //���ҩʽ��װ����������
                break;
            case 2:
                if (energy >= 12000 && (ships[shipno - 1]->weaponType == THUAI7::WeaponType::LaserGun || ships[shipno - 1]->weaponType == THUAI7::WeaponType::NullWeaponType))
                    api.InstallModule(shipno, THUAI7::ModuleType::ModulePlasmaGun);
                break;
            case 3:
                if (energy >= 13000 && (ships[shipno - 1]->weaponType == THUAI7::WeaponType::LaserGun || ships[shipno - 1]->weaponType == THUAI7::WeaponType::PlasmaGun || ships[shipno - 1]->weaponType == THUAI7::WeaponType::NullWeaponType))
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleShellGun);
                break;
            case 4:
                if (energy >= 18000 && ships[shipno - 1]->weaponType != THUAI7::WeaponType::MissileGun)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleMissileGun);
                break;
            case 5:
                if (energy >= 24000 && ships[shipno - 1]->weaponType != THUAI7::WeaponType::ArcGun)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleArcGun);
                break;
            default:
                break;
        }
    }
    else
        return;
}

void Military_Module_armour(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0 )
    {
        return;
    }
    auto armortype = ships[shipno - 1]->armorType;
    int energy = api.GetEnergy();
    if (ShipTypeDict[shipno - 1] != THUAI7::ShipType::NullShipType)
    {
        switch (type)  
        {
            case 1:
                if (armortype==THUAI7::ArmorType::NullArmorType&&energy>=6000)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleArmor1);
                break;
            case 2:
                if (energy >= 12000 && (armortype == THUAI7::ArmorType::NullArmorType||armortype == THUAI7::ArmorType::Armor1))
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleArmor2);
                break;
            case 3:
                if (energy >= 18000 && armortype != THUAI7::ArmorType::Armor3)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleArmor3);
                break;
            default:
                break;
        }
    }
    else
        return;
}

void Military_Module_shield(ITeamAPI& api, int shipno, int type)
{
    auto ships = api.GetShips();
    int size = ships.size();
    if (shipno > size || size == 0)
    {
        return;
    }
    auto shieldtype = ships[shipno - 1]->shieldType;
    int energy = api.GetEnergy();
    if (ShipTypeDict[shipno - 1] != THUAI7::ShipType::NullShipType)
    {
        switch (type)
        {
            case 1:
                if (shieldtype == THUAI7::ShieldType::NullShieldType && energy >= 6000)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleShield1);
                break;
            case 2:
                if (energy >= 12000 && (shieldtype == THUAI7::ShieldType::NullShieldType || shieldtype == THUAI7::ShieldType::Shield1))
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleShield2);
                break;
            case 3:
                if (energy >= 18000 && shieldtype != THUAI7::ShieldType::Shield3)
                    api.InstallModule(shipno, THUAI7::ModuleType::ModuleShield3);
                break;
            default:
                break;
        }
    }
    else
        return;
}

void Strategy_Military_Steal(IShipAPI& api)
{
    while (attack(api))
    {
        GoPlace(api, home_vec[1].x_4p, home_vec[1].y_4p);  // ͵��
    }
}

void Strategy_Military_Guard(IShipAPI& api)
{
    int mytID = api.GetSelfInfo()->teamID;
    auto shadowforguard = findclosest(api,THUAI7::PlaceType::Shadow, home_vec[mytID].x, home_vec[mytID].y);
    std::pair<int, int> myplace = {api.GridToCell(api.GetSelfInfo()->x), api.GridToCell(api.GetSelfInfo()->y)};
    if (myplace == shadowforguard)
    {
        AttackShip(api);
    }
    else
    {
        GoPlace_Loop(api, shadowforguard.first, shadowforguard.second);
    }
}

bool Chase(IShipAPI& api,int last_seen_gridx,int last_seen_gridy)
{
    if (Update_Enemy(api))
    {
        return true;
    }
    int number = Enemy_Attack_Index(api);
    GoPlace_Loop(api, last_seen_gridx, last_seen_gridy);
    if (once_met_enemys)
    {
        once_met_enemys = false;
        return true;
    }
    if (Update_Enemy(api))
    {
        int number = Enemy_Attack_Index(api);
        return Chase(api, enemy_vec[number].gridx, enemy_vec[number].gridy);
    }
    else
        return false;
}

void Go_Recover(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int intendedhp = 3000;
    int hp = selfinfo->hp;
    auto shiptype = selfinfo->shipType;
    if (shiptype == THUAI7::ShipType::CivilianShip)
    {
        intendedhp = 3000;
    }
    else if (shiptype == THUAI7::ShipType::MilitaryShip)
    {
        intendedhp = 4000;
    }
    else
    {
        intendedhp = 12000;
    }
    std::vector<my_Construction>& temp = construction_vec;
    for (int i = 0; i < temp.size(); i++)
    {
        // ��community��ȥcommunity��Ѫ
        if (temp[i].type == THUAI7::ConstructionType::Community)
        {
            bool b_temp = GoPlace_Loop(api, temp[i].x_4c, temp[i].y_4c);

            if (b_temp == true)
            {
                int count = 0;
                int round = 0;
                while (hp < intendedhp && round < 40)
                {
                    api.Recover(100);
                    count++;
                    if (count % 10 == 0)
                    {
                        count = 0;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        hp = api.GetSelfInfo()->hp;
                        round++;
                    }
                }
            }
        }
    }

    // û��community�ͻؼһ�Ѫ
    if (api.GetHomeHp())
    {
        GoPlace_Loop(api, home_vec[0].x + 1, home_vec[0].y);
    }
    return;
}

bool Under_Attack(IShipAPI& api)
{
    auto info0 = api.GetSelfInfo();
    int hp0 = info0->hp;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto info1 = api.GetSelfInfo();
    int hp1 = info1->hp;
    if (info1 < info0)
    {
        return true;
    }
    return false;
}

bool Attack_Loop_Cons(IShipAPI& api, double angle, my_Construction cons)
{  // ����ѭ��������ʽ ����ÿ����һ�ζ�Ҫ���½��������˷����ж�ʱ��

    int count = 0;
    int round = 0;
    int team = -1;
    int hp = 0;
    if (api.GetConstructionState(cons.x, cons.y).has_value())
    {
        auto state = api.GetConstructionState(cons.x, cons.y);
        int team = state->teamID;
        hp = state->hp;
        auto type = state->constructionType;
        if (type == THUAI7::ConstructionType::Fort && hp > 8000)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    if (team == api.GetSelfInfo()->teamID)
    {
        return false;
    }

    while (hp > 0 && round < 50)
    {
        api.Attack(angle);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        count++;
        if (count % 10 == 0)
        {
            round++;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (api.GetConstructionState(cons.x, cons.y).has_value())
            {
                hp = api.GetConstructionState(cons.x, cons.y)->hp;
                cons.HP = hp;
            }
        }
    }

    api.Print("Attack Loop Terminated! (" + std::to_string(cons.x) + "," + std::to_string(cons.y) + ")\n");
    if (hp == 0)
    {
        return true;
    }

    return false;
}

bool Attack_Cons(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int x = selfinfo->x;
    int y = selfinfo->y;

    std::vector<my_Construction>& temp = construction_vec;
    int size = temp.size();
    bool judge = false;

    for (int i = 0; i < size; i++)
    {
        if (api.HaveView(temp[i].x, temp[i].y))
        {
            if (temp[i].group != 1 && (api.GetConstructionState(temp[i].x, temp[i].y).has_value()))
            {
                if (api.GetConstructionState(temp[i].x, temp[i].y)->hp == 0)
                {
                    continue;
                }
                // �ж�Ϊ�з� �����й���
                temp[i].group = 2;
                double gridx = api.CellToGrid(temp[i].x);
                double gridy = api.CellToGrid(temp[i].y);

                if (gridx == x)
                {
                    if (gridy > y)
                    {
                        judge = Attack_Loop_Cons(api, pi / 2, temp[i]);
                    }
                    else
                    {
                        judge = Attack_Loop_Cons(api, 3 * pi / 2, temp[i]);
                    }
                }
                else if (gridy == y)
                {
                    if (gridx > x)
                    {
                        judge = Attack_Loop_Cons(api, 0, temp[i]);
                    }
                    else
                    {
                        judge = Attack_Loop_Cons(api, pi, temp[i]);
                    }
                }
                else if (gridx - x > 0 && gridy - y > 0)
                {
                    double angle = atan((gridy - y) / (gridx - x));
                    judge = Attack_Loop_Cons(api, angle, temp[i]);
                }
                else if (gridx - x > 0 && gridy - y < 0)
                {
                    double angle = atan((gridy - y) / (gridx - x)) + 2 * pi;
                    judge = Attack_Loop_Cons(api, angle, temp[i]);
                }
                else
                {
                    double angle = atan((gridy - y) / (gridx - x)) + pi;
                    judge = Attack_Loop_Cons(api, angle, temp[i]);
                }
            }
        }
    }
    return judge;
}

const double Count_Angle(IShipAPI& api, int tar_gridx, int tar_gridy)
{
    auto selfinfo = api.GetSelfInfo();
    int myx = selfinfo->x;
    int myy = selfinfo->y;
    double disx = tar_gridx - myx;
    double disy = tar_gridy - myy;
    double angle;
    if (disx == 0)
        if (disy > 0)
            angle = pi / 2;
        else
            angle = -pi / 2;
    else if (disy == 0)
        if (disx > 0)
            angle = 0;
        else
            angle = pi;
    else if (disx > 0 && disy > 0)
        angle = atan(disy / disx);
    else if (disx > 0 && disy < 0)
        angle = atan(disy / disx) + 2 * pi;
    else
        angle = atan(disy / disx) + pi;
    return angle;
}

void Greedy_Resource_Limit(IShipAPI& api, int limit)
{
Begin:
    auto selfinfo = api.GetSelfInfo();
    int ID = selfinfo->playerID;
    if (ship_re[ID - 1] >= limit)
    {
        return;
    }
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int size = resource_vec.size();
    if (size == 0)
    {
        api.Print("Please Get Resource ! ");
        return;
    }
    // minimum��ʾ·����Сֵ
    int minimum = 1000;
    // order��ʾ��С��Ӧ�ı��
    int order = -1;
    int x;
    int y;

    Update_Map(api);
    for (int i = 0; i < size; i++)
    {
        x = resource_vec[i].x;
        y = resource_vec[i].y;

        // Greedy�㷨�ҵ����Լ��������Դ
        if (resource_vec[i].produce == false)
        {
            auto path = findShortestPath(Map_grid, {cellx, celly}, {resource_vec[i].x_4p, resource_vec[i].y_4p}, api);
            int size = path.size();
            if (size < minimum && size > 0)
            {
                minimum = size;
                order = i;
            }
        }
    }
    if (order == -1)
    {
        // order==-1��ʾδ���ַ���Ҫ���
        api.Print("Finished!");
        return;
    }

    // ǰ������
    x = resource_vec[order].x;
    y = resource_vec[order].y;
    int temp = false;
    if (api.GetSelfInfo()->playerID % 2 == 1)
    {
        temp = GoPlace_Loop(api, resource_vec[order].x_4p, resource_vec[order].y_4p);
    }
    else
    {
        temp = GoPlace_Loop(api, resource_vec[order].x_4p2, resource_vec[order].y_4p2);
    }
    if (temp == false)
    {
    }
    int state = api.GetResourceState(x, y);

    int count = 0;
    while (state > 0)
    {  // ֻҪ����ʣ����Դ�Ϳ���
        api.Produce();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        count++;
        if (count % 10 == 0)
        {
            // ÿʮ�ν���һ���ж��뷵��
            api.Wait();
            Judge_4_Civil(api);
            state = api.GetResourceState(x, y);
            resource_vec[order].HP = state;
            count = 0;
        }
    }

    if (state == 0 && ship_re[ID - 1] <= limit)
    {
        ship_re[ID - 1]++;
        resource_vec[order].produce = true;
        goto Begin;
    }
    return;
}

std::pair<int, int> findclosest(IShipAPI& api,THUAI7::PlaceType type, int des_x, int des_y)
{
    std::pair<int, int> closest;
    auto map = api.GetFullMap();
    for (int i = 0; i<49 ; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            if (des_x + j < 50 && des_y+(i-j)<50 && map[des_x + j][des_y+(i-j)] == type)
            {
                closest.first = des_x + j;
                closest.second = des_y+(i-j);
                return closest;
            }
            if (des_x + j < 50 && des_y - (i - j)>0 &&map[des_x + j][des_y - (i - j)] == type)
            {
                closest.first = des_x + j;
                closest.second = des_y-(i-j);
                return closest;
            }
            if (des_x - j >0 && des_y + (i - j) <50 && map[des_x - j][des_y + (i - j)] == type)
            {
                closest.first = des_x - j;
                closest.second = des_y + (i - j);
                return closest;
            }
            if (des_x - j >0 && des_y - (i - j) > 0 && map[des_x - j][des_y - (i - j)] == type)
            {
                closest.first = des_x - j;
                closest.second = des_y - (i - j);
                return closest;
            }
        }
    }
    return closest;
}

int Judge_4_Civil(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    auto enemyinfo = api.GetEnemyShips();
    auto ships = api.GetShips();

    int cellx = api.GridToCell(selfinfo->x);
    int celly = api.GridToCell(selfinfo->y);

    int size = construction_vec.size();

    if (enemyinfo.size() != 0)
    {
        api.Print("Find Enemy ships!");
        if (api.GetSelfInfo()->weaponType == THUAI7::WeaponType::NullWeaponType)
        {
            // hide(api);
            int enemyx = api.GridToCell(enemyinfo[0]->x);
            int enemyy = api.GridToCell(enemyinfo[0]->y);
            if (ships.size() < 3)
            {
                // û�п�����ȵ��Ѿ�
                return 0;
            }
            if (enemyx < 10 && enemyy < 10)
            {
                api.SendBinaryMessage(3, "10" + std::to_string(enemyx) + "0" + std::to_string(enemyy));
            }
            else if (enemyx < 10)
            {
                api.SendBinaryMessage(3, "10" + std::to_string(enemyx) + std::to_string(enemyy));
            }
            else if (enemyy < 10)
            {
                api.SendBinaryMessage(3, "1" + std::to_string(enemyx) + "0" + std::to_string(enemyy));
            }
            else
            {
                api.SendBinaryMessage(3, "1" + std::to_string(enemyx) + std::to_string(enemyy));
            }
            api.Print("Message Send!");
        }
    else
    {
        AttackShip(api);
    }
    }
    return 0;
}

void Update_Cons(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int cellx = api.GridToCell(selfinfo->x);
    int celly = api.GridToCell(selfinfo->y);

    int size = construction_vec.size();
    for (int i = 0; i < size; i++)
    {
        if ((construction_vec[i].x - cellx) * (construction_vec[i].x - cellx) + (construction_vec[i].y - celly) * (construction_vec[i].y - celly) < 64)
        {
            if (!api.GetConstructionState(construction_vec[i].x, construction_vec[i].y).has_value())
            {
                continue;
            }
            auto info = api.GetConstructionState(construction_vec[i].x, construction_vec[i].y);
            construction_vec[i].group = info->teamID;
            construction_vec[i].HP = info->hp;
            if (construction_vec[i].HP > 5000)
            {
                construction_vec[i].build = true;
            }
        }
    }
    return;
}

void Judge_4_Base(ITeamAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    auto ships = api.GetShips();
    int hp0 = api.GetHomeHp();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    int hp1 = api.GetHomeHp();
    auto enemyinfo = api.GetEnemyShips();

    if (hp1 < hp0)
    {
        if (ships.size() >= 3)
        {
            if (enemyinfo.size() > 0)
            {
                int enemyx = api.GridToCell(enemyinfo[0]->x);
                int enemyy = api.GridToCell(enemyinfo[0]->y);
                if (enemyx < 10 && enemyy < 10)
                {
                    api.SendBinaryMessage(3, "10" + std::to_string(enemyx) + "0" + std::to_string(enemyy));
                }
                else if (enemyx < 10)
                {
                    api.SendBinaryMessage(3, "10" + std::to_string(enemyx) + std::to_string(enemyy));
                }
                else if (enemyy < 10)
                {
                    api.SendBinaryMessage(3, "1" + std::to_string(enemyx) + "0" + std::to_string(enemyy));
                }
                else
                {
                    api.SendBinaryMessage(3, "1" + std::to_string(enemyx) + std::to_string(enemyy));
                }
                return;
            }
            api.SendBinaryMessage(3, "0");
        }
    }
    api.Wait();
    return;
}

void Base_Build_Ship(ITeamAPI& api, int birthdes)
{
    auto ships = api.GetShips();
    int civil_num = 0;
    int milit_num = 0;
    for (int i = 0; i < ships.size(); i++)
    {
        if (ships[i]->shipType == THUAI7::ShipType::CivilianShip)
        {
            civil_num++;
        }
        else if (ships[i]->shipType == THUAI7::ShipType::MilitaryShip || ships[i]->shipType == THUAI7::ShipType::FlagShip)
        {
            milit_num++;
        }
    }
    if (civil_num == 1)
    {
        Build_Ship(api, 2, birthdes);
    }
    else if (civil_num == 2 && milit_num == 0)
    {
        Build_Ship(api, 3, birthdes);
    }
    else if (civil_num == 2 && milit_num == 1)
    {
        Build_Ship(api, 4, birthdes);
    }
    else if (civil_num == 0)
    {
        Build_Ship(api, 1, birthdes);
        Build_Ship(api, 2, birthdes);
    }
}

void Construction_Attack(IShipAPI& api)
{
    int size = construction_vec.size();
    auto selfinfo = api.GetSelfInfo();

    if (size == 0 || selfinfo->weaponType == THUAI7::WeaponType::NullWeaponType)
    {
        return;
    }
    else
    {
        for (int j = 0; j < size; j++)
        {
            if (construction_vec[j].build == false && ((selfinfo->teamID == 0 && construction_vec[j].x > 25) || (selfinfo->teamID == 1 && construction_vec[j].x < 23)))
            {
                GoPlace_Loop(api, construction_vec[j].x_4c, construction_vec[j].y_4c);
                if (construction_vec[j].x_4c - construction_vec[j].x == 1)
                {
                    Attack_Loop_Cons(api, pi, construction_vec[j]);
                }
                else if (construction_vec[j].x_4c - construction_vec[j].x == -1)
                {
                    Attack_Loop_Cons(api, 0, construction_vec[j]);
                }
                else if (construction_vec[j].y_4c - construction_vec[j].y == 1)
                {
                    Attack_Loop_Cons(api, 3 * pi / 2, construction_vec[j]);
                }
                else
                {
                    Attack_Loop_Cons(api, pi / 2, construction_vec[j]);
                }
            }
            construction_vec[j].build = true;
        }
        for (int j = 0; j < size; j++)
        {
            if (construction_vec[j].build == false && ((selfinfo->teamID == 0 && construction_vec[j].x < 23) || (selfinfo->teamID == 1 && construction_vec[j].x > 25)))
            {
                GoPlace_Loop(api, construction_vec[j].x_4c, construction_vec[j].y_4c);
                if (construction_vec[j].x_4c - construction_vec[j].x == 1)
                {
                    Attack_Loop_Cons(api, pi, construction_vec[j]);
                }
                else if (construction_vec[j].x_4c - construction_vec[j].x == -1)
                {
                    Attack_Loop_Cons(api, 0, construction_vec[j]);
                }
                else if (construction_vec[j].y_4c - construction_vec[j].y == 1)
                {
                    Attack_Loop_Cons(api, 3 * pi / 2, construction_vec[j]);
                }
                else
                {
                    Attack_Loop_Cons(api, pi / 2, construction_vec[j]);
                }
            }
            construction_vec[j].build = true;
        }
    }
    return;
}

bool Advantage(IShipAPI& api)
{
    auto gameinfo = api.GetGameInfo();
    auto selfinfo = api.GetSelfInfo();
    if (selfinfo->teamID == 0)
    {
        if (gameinfo->redHomeHp > 0)
        {
            if (gameinfo->redScore - gameinfo->blueScore >= 100000 || gameinfo->blueHomeHp == 0)
            {
                return true;
            }
        }
    }
    else
    {
        if (gameinfo->blueHomeHp > 0)
        {
            if (gameinfo->blueScore - gameinfo->redScore >= 100000 || gameinfo->blueHomeHp == 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool Update_Enemy(IShipAPI& api)
{
    // ����false == û�е���
    // ����true == ���ֵ���
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    auto selfinfo = api.GetSelfInfo();
    auto enemy = api.GetEnemyShips();
    int size = enemy.size();
    enemy_vec.clear();
    if (size == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            enemy_vec.push_back({enemy[i]->shipType, enemy[i]->hp, enemy[i]->x, enemy[i]->y, enemy[i]->weaponType});
            api.Print("This is Enemy Info:" + std::to_string(enemy_vec[0].gridx) + "," + std::to_string(enemy_vec[0].gridy) + "\n");
        }
    }
    return true;
}

int Enemy_Attack_Index(IShipAPI& api)
{  // ���ص�ǰӦ�������ĵ�����vec�е�����
    int size = enemy_vec.size();
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;

    if (size == 0)
    {
        return -1;
    }
    else
    {
        int array[4];
        int distance[4];
        for (int i = 0; i < 4; i++)
        {
            array[i] = -1;
            distance[i] = -1;
        }
        int flag = 0;
        for (int i = 0; i < size; i++)
        {
            if (enemy_vec[i].weapon != THUAI7::WeaponType::NullWeaponType)
            {
                array[flag] = i;
                flag++;
            }
        }
        if (flag == 0)
        {
            return 0;
        }
        else if (flag == 1)
        {
            return array[0];
        }
        for (int j = 0; j < flag; j++)
        {
            distance[j] = sqrt((gridx - enemy_vec[j].gridx) * (gridx - enemy_vec[j].gridx) + (gridy - enemy_vec[j].gridy) * (gridy - enemy_vec[j].gridy));
        }
        int minimum = distance[0];
        int count = 0;
        for (int k = 1; k < flag; k++)
        {
            if (distance[k] > 0 && distance[k] < minimum)
            {
                count = k;
                minimum = distance[k];
            }
        }
        return count;
    }
    return -1;
}

void Attack_Base(IShipAPI& api)
{
    auto gameinfo = api.GetGameInfo();
    auto selfinfo = api.GetSelfInfo();
    int teamid = selfinfo->teamID;

    int count = 0;
    int round = 0;

    if (teamid == 0)
    {
        int HP = gameinfo->blueHomeHp;
        if (gameinfo->blueHomeHp == 0)
        {
            return;
        }
        else
        {
            GoPlace_Dis_Loop(api, home_vec[1].x - 1, home_vec[1].y + 1);
            auto selfinfo = api.GetSelfInfo();
            if (api.GridToCell(selfinfo->x) == home_vec[1].x_4p && api.GridToCell(selfinfo->y) == home_vec[1].y_4p)
            {
                double disx = api.CellToGrid(home_vec[1].x_4p) - selfinfo->x;
                double disy = api.CellToGrid(home_vec[1].y_4p) - selfinfo->y;
                while (round < 30 && HP > 0)
                {
                    api.Attack(Count_Angle(api, disx, disy));
                    count++;

                    if (count == 10)
                    {
                        count = 0;
                        round++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(400));
                        HP = api.GetGameInfo()->blueHomeHp;
                    }
                }
            }
        }
    }
    else
    {
        int HP = gameinfo->redHomeHp;
        if (HP == 0)
        {
            return;
        }
        else
        {
            GoPlace_Loop(api, home_vec[1].x + 1, home_vec[1].y - 1);
            auto selfinfo = api.GetSelfInfo();
            if (api.GridToCell(selfinfo->x) == home_vec[1].x_4p && api.GridToCell(selfinfo->y) == home_vec[1].y_4p)
            {
                double disx = api.CellToGrid(home_vec[1].x_4p) - selfinfo->x;
                double disy = api.CellToGrid(home_vec[1].y_4p) - selfinfo->y;
                while (round < 30 && HP > 0)
                {
                    api.Attack(Count_Angle(api, disx, disy));
                    count++;

                    if (count == 10)
                    {
                        count = 0;
                        round++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(400));
                        HP = api.GetGameInfo()->redHomeHp;
                    }
                }
            }
        }
    }
    return;
}

bool GoPlace_Dis(IShipAPI& api, int des_x, int des_y)
{
    auto selfinfo = api.GetSelfInfo();
    auto weapon = selfinfo->weaponType;
    int intenddis = 4000;
    if (weapon == THUAI7::WeaponType::LaserGun || weapon == THUAI7::WeaponType::PlasmaGun || weapon == THUAI7::WeaponType::ShellGun)
    {
        intenddis = 4000;
    }
    else if (weapon == THUAI7::WeaponType::NullWeaponType)
    {
        intenddis = 0;
        return false;
    }
    else
    {
        intenddis = 6000;
    }
Restart:
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    selfinfo = api.GetSelfInfo();
    int cur_gridx = selfinfo->x;
    int cur_gridy = selfinfo->y;
    int cur_x = api.GridToCell(cur_gridx);
    int cur_y = api.GridToCell(cur_gridy);
    double speed = 3.0;
    int des_gx = api.CellToGrid(des_x);
    int des_gy = api.CellToGrid(des_y);

    if (selfinfo->shipType == THUAI7::ShipType::CivilianShip)
    {
        speed = SPEED_CIVIL_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::FlagShip)
    {
        speed = SPEED_FLAG_MS;
    }
    else if (selfinfo->shipType == THUAI7::ShipType::MilitaryShip)
    {
        speed = SPEED_MILIT_MS;
    }

    Point start = {cur_x, cur_y};
    Point end = {des_x, des_y};

    std::vector<Point> path = findShortestPath(Map_grid, start, end, api);
    int path_size = path.size();

    for (int i = 0; i < path_size - 1; i++)
    {
        direction[i] = Point{
            path[i + 1].x - path[i].x,
            path[i + 1].y - path[i].y
        };
    }
    for (int j = 0; j < path_size - 1; j++)
    {
        if (j % 10 == 0 && j > 0)
        {  // ÿ�ƶ�ʮ�ν���һ��GoCell
            GoCell(api);
            Judge_4_Civil(api);
            if (api.GridToCell(api.GetSelfInfo()->x) == cur_x && api.GridToCell(api.GetSelfInfo()->y) == cur_y)
            {  // û�б仯����ס�ˣ�����������
                goto Restart;
            }
        }
        if (path_size - j < 10)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto selfinfo = api.GetSelfInfo();
            int cur_x = selfinfo->x;
            int cur_y = selfinfo->y;

            if (sqrt((cur_x - des_gx) * (cur_x - des_gx) + (cur_y - des_gy) * (cur_y - des_gy)) <= intenddis)
            {
                return true;
            }
        }
        if (direction[j].x == 1 && direction[j].y == 1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == 1 && direction[j].y == -1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 7 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1 && direction[j].y == 1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 3 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1 && direction[j].y == -1)
        {
            AttackShip(api);
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            api.Move(250 * sqr2 / speed, 5 * pi / 4);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        else if (direction[j].x == -1)
        {
            AttackShip(api);
            api.MoveUp(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (direction[j].x == 1)
        {
            AttackShip(api);
            api.MoveDown(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else if (direction[j].y == -1)
        {
            AttackShip(api);
            api.MoveLeft(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
        else
        {
            AttackShip(api);
            api.MoveRight(1000 / speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
    }

    return false;
}

void GoPlace_Dis_Loop(IShipAPI& api, int des_x, int des_y)
{
    int temp = false;
    int round = 0;
    while (temp == false && round < 10)
    {
        temp = GoPlace_Dis(api, des_x, des_y);
        round++;
    }
}