#include <vector>
#include <thread>
#include <array>
#include <queue>
#include <map>
#include "AI.h"
#include "constants.h"
#include <math.h>

#define pi 3.141592653589
// ע�ⲻҪʹ��conio.h��Windows.h�ȷǱ�׼��
// Ϊ����play()�ڼ�ȷ����Ϸ״̬�����£�Ϊ����ֻ��֤��Ϸ״̬�ڵ�����ط���ʱ�����£�����һ֡����һ��
extern const bool asynchronous = true;

// ѡ����Ҫ���ν�player1��player4�Ĵ����������ﶨ��
extern const std::array<THUAI7::ShipType, 4> ShipTypeDict = {
    THUAI7::ShipType::CivilianShip,
    THUAI7::ShipType::CivilianShip,
    THUAI7::ShipType::MilitaryShip,
    THUAI7::ShipType::FlagShip,
};

// ������AI.cpp�ڲ����������뺯��

// ��������
const int map_size = 50;
int dx[] = {-1, 0, 1, 0};
int dy[] = {0, -1, 0, 1};
int count1 = 0;
int count2 = 0;
bool temp1 = false;
const double SPEED_CIVIL_MS = 3.00;
const double SPEED_MILIT_MS = 2.80;
const double SPEED_FLAG_MS = 2.70;

struct Point
{
    int x, y;
};
struct Point direction[1000];


// �����ĸ��࣬����ִ����ز���
class my_Resource
{
public:
    int HP=16000;
    int x;
    int y;
    // ���ɵ�λ
    int x_4p;
    int y_4p;
    my_Resource(int x_, int y_,int x4,int y4)
    {
        x = x_;
        y = y_;
        x_4p = x4;
        y_4p = y4;
    }
};

class my_Construction
{
public:
    THUAI7::ConstructionType type=THUAI7::ConstructionType::NullConstructionType;
    int x;
    int y;

    // �����λ
    int x_4c;
    int y_4c;
    int HP=0;
    // 0Ϊ�� 1Ϊ���� 2Ϊ�з�
    int group=0;

    my_Construction(int i, int j,int i_4c,int j_4c)
    {
        x = i;
        y = j;
        x_4c = i_4c;
        y_4c = j_4c;
    }
};

class my_Home
{
public:
    int x;
    int y;
    int HP=10000;

    // 1���� 2�з�
    int group=0;

    my_Home(int i, int j, int gp)
    {
        x = i;
        y = j;
        group = gp;
        
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

std::vector<my_Resource> resource_vec;
std::vector<my_Construction> construction_vec;
std::vector<my_Home> home_vec;
std::vector<my_Wormhole> wormhole_vec;

std::vector<std::vector<int>> Map_grid(map_size, std::vector<int>(map_size, 1));
struct NeedModule
{
    int number;
    THUAI7::ModuleType moduletype;
};
NeedModule modl1;




// �����ǵ��õĺ����б�
void Judge(IShipAPI& api);                         // �ж�Ӧ�������ĸ�����(����/��ȡ��Դ��)
void Base_Operate(ITeamAPI& api);                  // ���صĲ���
void AttackShip(IShipAPI& api);                    // �����з���ֻ
void Install_Module(ITeamAPI& api, int number, int type);  // Ϊ��ֻ��װģ�� 1:Attack 2:Construct 3:Comprehensive
bool GoCell(IShipAPI& api);                        // �ƶ���cell����


// ������Ѱ·��صĺ���
bool isValid(IShipAPI& api, int x, int y);
std::vector<std::vector<int>> Get_Map(IShipAPI& api);                            // �õ�һ����άvector��������ͼ�Ͽ���/��������Ϣ
const std::vector<Point> findShortestPath(const std::vector<std::vector<int>>& grid, Point start, Point end, IShipAPI& api);  // �����������Ѱ·
bool GoPlace(IShipAPI& api, int des_x, int des_y);
void GoPlace_Loop(IShipAPI& api,int des_x,int des_y);
bool Path_Release(std::vector<Point> Path, IShipAPI& api,int count); //ʵ��·��(δʹ��)

// �����ǿ�����Դ��������غ���

// �����������м�����Դ
void Get_Resource(IShipAPI& api);

// ��ѡ����λ�ý���ѡ���Ľ�����
void Build_Specific(IShipAPI& api, THUAI7::ConstructionType type, int number);

// ������λ�ý���ѡ���Ľ�����
void Build_ALL(IShipAPI& api, THUAI7::ConstructionType type);

// �����Ǵ�Ӫ������غ���

// ��װ����ģ��
void Produce_Module(ITeamAPI& api, int shipno, int limit,int type=3);

// ��װ����ģ��
void Construct_Module(ITeamAPI& api, int shipno, int limit, int type = 3);

// ��������
void Build_Ship(ITeamAPI& api, int shipno, int birthdes);





void AI::play(IShipAPI& api)
{
    if (api.GetFrameCount() == 1)
    {
        // ���ҽ��� ֡��Ϊ 1  ����Get_Map 
        // ��ʡ����ʱ��
        Get_Map(api);
    }
    if (this->playerID == 1)
    {
        Get_Resource(api);
        api.PrintSelfInfo();
    }
    else if (this->playerID == 2)
    {
        Build_ALL(api, THUAI7::ConstructionType::Factory);
        api.PrintSelfInfo();
    }
    else if (this->playerID == 3)
    {
        api.PrintSelfInfo();
    }
    else if (this->playerID == 4)
    {
        api.PrintSelfInfo();
        GoPlace_Loop(api, home_vec[1].x + 1, home_vec[1].y);
        api.Attack(pi);
    }
}

void AI::play(ITeamAPI& api)  // Ĭ��team playerID Ϊ0
{
    api.PrintSelfInfo();
    api.PrintTeam();
    Produce_Module(api, 1, 8000, 3);
    api.Wait();
    Build_Ship(api, 2, 0);
    Build_Ship(api, 3, 0);
    Build_Ship(api, 4, 0);
}







bool GoCell(IShipAPI& api)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    double speed;
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

    //����һЩ����CELL������Ϣ
    std::string strx = std::to_string(cellx);
    std::string stry = std::to_string(celly);
    api.Print("GoCell Called");
    api.Print("This is current cell_X");
    api.Print(strx);
    api.Print("This is current cell_Y");
    api.Print(stry);

    int Gcellx = 1000 * cellx + 500;
    int Gcelly = 1000 * celly + 500;

    if (abs(Gcellx - gridx) <= 30 && abs(Gcelly - gridy) <= 30)
    {
        api.Print("Already Finished!");
        return true;
    }
    else
    {
        if (Gcellx < gridx)
        {
            api.MoveUp((gridx - Gcellx)/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else if (Gcellx > gridx)
        {
            api.MoveDown((Gcellx - gridx)/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (Gcelly < gridy)
        {
            api.MoveLeft((gridy - Gcelly)/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else if (Gcelly > gridy)
        {
            api.MoveRight((Gcelly - gridy)/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    api.Wait();
    selfinfo = api.GetSelfInfo();
    gridx = selfinfo->x;
    gridy = selfinfo->y;
    cellx = api.GridToCell(gridx);
    celly = api.GridToCell(gridy);
    Gcellx = 1000 * cellx + 500;
    Gcelly = 1000 * celly + 500;
    
    if (abs(Gcellx - gridx) <= 30 && abs(Gcelly - gridy) <= 30)
    {
        api.PrintShip();
        api.PrintSelfInfo();
        api.Print("GoCell Finished !");
        return true;
    }
    else
    {
        api.PrintShip();
        api.PrintSelfInfo();
        api.Print("GoCell Did not Finish");
        return false; 
    }
}




void Base_Operate(ITeamAPI& api)
{
    auto selfships = api.GetShips();
    int count = 0;
    if (selfships[count] != nullptr)
    {
        // ���㼺����ֻ��Ŀ
        count++;
    }
    if (count < 2)
    {
        // С�ڶ���ʱ������һ���µ���װ��
        api.BuildShip(THUAI7::ShipType::MilitaryShip, 0);
    }
    else if (count == 2)
    {
        api.BuildShip(THUAI7::ShipType::CivilianShip, 0);
    }
    else if (count == 3)
    {
        api.BuildShip(THUAI7::ShipType::FlagShip, 0);
    }
    return;
}

void Install_Module(ITeamAPI& api, int number, int type)
{
    // ����number��ŵĴ�ֻ��װ��Ӧ��װ��
    auto selfships = api.GetShips();
    if (type == 1)
    {
        // һ������װ��(Attack)
        api.InstallModule(number, THUAI7::ModuleType::ModuleMissileGun);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor2);
    }
    else if (type == 2)
    {
        // ��������װ��(Construct)
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        api.InstallModule(number, THUAI7::ModuleType::ModuleConstructor2);
    }
    else if (type == 3)
    {
        // ��������װ��(Comprehensive)
        api.InstallModule(number, THUAI7::ModuleType::ModuleArcGun);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        api.InstallModule(number, THUAI7::ModuleType::ModuleArmor1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        api.InstallModule(number, THUAI7::ModuleType::ModuleConstructor1);
    }
    return;
}

void AttackShip(IShipAPI& api)
{
    // �����з���ֻ�ĺ���
    int gridx = api.GetSelfInfo()->x;
    int gridy = api.GetSelfInfo()->y;

    auto Enemys = api.GetEnemyShips();
    int size = Enemys.size();

    if (size == 0 || api.GetSelfInfo()->weaponType == THUAI7::WeaponType::NullWeaponType)
    {
        // �޹����������û��������xs��������(1,47)�������Ϊʲô�Ҳ����������ؼҵ�λ�ã���
        GoPlace(api, 1, 47);
        return;
    }

    // atan��math.h����ĺ���(����������)
    // ���ڵõ��з���ֻ�ķ�λ
    auto Enemy0x = Enemys[0]->x;
    auto Enemy0y = Enemys[0]->y;
    int dis0x = Enemy0x - gridx;
    int dis0y = Enemy0y - gridy;
    double angle0 = atan(dis0y / dis0x);
    double distance0 = dis0y * dis0y + dis0x * dis0y;

    // ������������Ұ��
    if (Enemys[1] != nullptr)
    {
        auto Enemy1x = Enemys[1]->x;
        auto Enemy1y = Enemys[1]->y;
        int dis1x = Enemy1x - gridx;
        int dis1y = Enemy1y - gridy;
        double angle1 = atan(dis1y / dis1x);
        double distance1 = dis1x * dis1x + dis1y * dis1y;

        // ���ȹ����ٽ��Ĵ�ֻ
        if (distance1 <= distance0)
        {
            api.Attack(angle1);
        }
        else
        {
            api.Attack(angle0);
        }
    }
    else
    {
        // ֻ��һ����
        api.Attack(angle0);
    }
}
void Judge(IShipAPI& api)
{
    // �����жϣ�����Ӧ������ʲô�������������ǻ�ȡ��Դ
    int gridx = api.GetSelfInfo()->x;
    int gridy = api.GetSelfInfo()->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);
    auto map = api.GetFullMap();
    int HP = api.GetSelfInfo()->hp;

    auto enemyships = api.GetEnemyShips();
    int totalenenmyhp = 0;
    for (int i = 0; enemyships[i] != nullptr; i++)
        totalenenmyhp += enemyships[i]->hp;
    // ������˿Ѫ���Σ���shadow��
    if (HP < 1000 && totalenenmyhp > HP)  // �������ܻ�Ҫ��
    {
        for (int i = cellx - 8 < 0 ? 0 : cellx - 8; i < (cellx + 9 > 50 ? 50 : cellx + 9); i++)
        {
            for (int j = celly - sqrt(8 * 8 - (i - cellx) * (i - cellx)) < 0 ? 0 : celly - sqrt(8 * 8 - (i - cellx) * (i - cellx)); j < (celly + sqrt(8 * 8 - (i - cellx) * (i - cellx)) + 1 > 50 ? 50 : celly + sqrt(8 * 8 - (i - cellx) * (i - cellx)) + 1); j++)
            {
                if (map[i][j] == THUAI7::PlaceType::Shadow)
                    GoPlace(api, i, j);
            }
        }
        // �����Ǹ�֪baseҪװװ����Ҫ����ȫ�ֱ���������Ϣ��base�ڵĺ��������ٽ����жϺ;��ߣ���Ϣ����Ҫ���߼����ж�
        modl1.number = api.GetSelfInfo()->playerID;
        modl1.moduletype = THUAI7::ModuleType::ModuleShield3;
    }
    // ����
    if (enemyships[0] != nullptr)
    {
        if (api.GetSelfInfo()->weaponType != THUAI7::WeaponType::NullWeaponType)
        {
            AttackShip(api);
            return;
        }
        else
            ;  // ��ʱ���ţ����������뺯�����������������˿Ѫ���κϲ�����
    }

    // ��Դ&����(To Be Edited)
    //  map ��������ͼ��(cell����) Ҳ����˵[i][j]��ȡֵ��0-49(50*50������)
    //  Ŀǰ������Χ����Ұ��Χһ��
    if (api.GetSelfInfo()->shipType == THUAI7::ShipType::CivilianShip || api.GetSelfInfo()->shipType == THUAI7::ShipType::FlagShip)
    {
        for (int i = cellx - 8 < 0 ? 0 : cellx - 8; i < (cellx + 9 > 50 ? 50 : cellx + 9); i++)
        {
            for (int j = celly - sqrt(8 * 8 - (i - cellx) * (i - cellx)) < 0 ? 0 : celly - sqrt(8 * 8 - (i - cellx) * (i - cellx)); j < (celly + sqrt(8 * 8 - (i - cellx) * (i - cellx)) + 1 > 50 ? 50 : celly + sqrt(8 * 8 - (i - cellx) * (i - cellx)) + 1); j++)
            {
                if (map[i][j] == THUAI7::PlaceType::Resource)
                {
                    // ���ﲻ���GetResourceState���׷�����ɶ��
                    // ��һ��int_32���ǲ�֪����ʾ��ʲô��˼(��ʾ����Hp,���ſ��ɼ�С)
                    // ����GetResourceStateҲֻ������Ұ��Χ��
                    if (api.GetResourceState(i, j) > 0)
                    {
                        // ǰ����λ�ø���
                        // ע�⣡����GoPlace������whileѭ��
                        bool temp = false;
                        for (int i_temp = 0; i_temp < 9 && temp == false; i_temp++)
                        {
                            temp = GoPlace(api, i - 1 + i_temp, j - 1 + i_temp);
                        }
                        api.Produce();
                        if (temp)
                            return;
                    }
                }
                // ȱ�жϲɼ����ǽ��������
                if (map[i][j] == THUAI7::PlaceType::Construction)  // ���������δ���졣����Ҫ��produce����attack
                {
                    // GetConstructionHp������Ѫ8000������6000������12000
                    // ���ڲ��жϽ���ʲô���㷨
                    if (api.GetConstructionHp(i, j) < 8000)  // ��װ�ǹ���
                    {
                        // ǰ����λ�ø���
                        bool temp = false;
                        for (int i_temp = 0; i_temp < 9 && temp == false; i_temp++)
                        {
                            temp = GoPlace(api, i - 1 + i_temp, j - 1 + i_temp);
                        }
                        api.Construct(THUAI7::ConstructionType::Factory);
                        if (temp)
                            return;
                    }
                }
            }
        }
    }
    // ȱ��װ��ģ�������������Ҳ��֪�����ĸ�������˵Ҫ����������д��
}


std::vector<std::vector<int>> Get_Map(IShipAPI& api)
{
    auto map = api.GetFullMap();
    auto selfinfo = api.GetSelfInfo();
    int cellx = api.GridToCell(selfinfo->x);
    for (int i = 0; i < map_size; i++)
    {
        for (int j = 0; j < map_size; j++)
        {
            if (map[i][j] == THUAI7::PlaceType::Space || map[i][j] == THUAI7::PlaceType::Shadow)
            {  // �ɾ����ĵص�Ϊ0��Ĭ��(������;��)Ϊ1
                Map_grid[i][j] = 0;
                //std::string str_1 = "(" + std::to_string(i) + "," + std::to_string(j) + ")";
                //api.Print(str_1);
                if (map[i][j] == THUAI7::PlaceType::Wormhole)
                {
                    auto hp = api.GetWormholeHp(i, j);
                    wormhole_vec.push_back(my_Wormhole(i,j,hp));

                    if (hp > 9000)
                    {
                        Map_grid[i][j] = 0;
                    }

                    std::string str_1 = "WormHole Found : (" + std::to_string(i) + "," + std::to_string(j) + ")\n";
                    std::string str_2 = "WormHole HP :" + std::to_string(hp) + "\n";
                    api.Print(str_1);
                    api.Print(str_2);
                }
            }
            else if (map[i][j] == THUAI7::PlaceType::Resource)
            {
                if (map[i+1][j] == THUAI7::PlaceType::Space || map[i+1][j] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i + 1,j));
                    continue;
                }
                else if (map[i - 1][j] == THUAI7::PlaceType::Space || map[i - 1][j] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i - 1, j));
                    continue;
                }
                else if (map[i][j + 1] == THUAI7::PlaceType::Space || map[i][j + 1] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i , j+1));
                    continue;
                }
                else if (map[i][j - 1] == THUAI7::PlaceType::Space || map[i][j - 1] == THUAI7::PlaceType::Shadow)
                {
                    resource_vec.push_back(my_Resource(i, j, i , j-1));
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
    if (cellx <= 25)
    {
        
        home_vec.push_back(my_Home(3, 46, 1));
        home_vec.push_back(my_Home(46, 3, 2));
        home_vec[0].HP = api.GetHomeHp();
    }
    else if (cellx >= 27)
    {
        home_vec.push_back(my_Home(46, 3, 1));
        home_vec.push_back(my_Home(3, 46, 2));
        home_vec[0].HP = api.GetHomeHp();
    }

    api.Print("The Map Has Already Got !");
    return Map_grid;
}

bool GoPlace(IShipAPI& api, int des_x, int des_y)
{
    api.Wait();
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
    bool temp = false;
    for (int j = 0; j < path_size - 1; j++)
    {
        if (j % 5 == 0)
        {   // ÿ�ƶ���ν���һ��GoCell
            GoCell(api);
        }
        if (direction[j].x == -1)
        {
            api.MoveUp(1000/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else if (direction[j].x == 1)
        {
            api.MoveDown(1000/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else if (direction[j].y == -1)
        {
            api.MoveLeft(1000/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else
        {
            api.MoveRight(1000/speed);
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
}
void GoPlace_Loop(IShipAPI& api, int des_x, int des_y)
{
    bool temp = false;
    int count = 0;
    while (temp == false && count < 10)
    {
        temp = GoPlace(api, des_x, des_y);
        count++;
    }
    if (temp == true)
    {
        api.Print("Arrived! Loop Terminated! ");
    }
    else
    {
        api.Print("Not Arrived Yet! Loop Terminated! ");
    }
}

bool isValid(IShipAPI& api, int x, int y)
{
    return (x >= 0 && x < map_size  && y >= 0 && y < map_size );
}

const std::vector<Point> findShortestPath(const std::vector<std::vector<int>>& grid, Point start, Point end, IShipAPI& api)
{
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
        for (int i = 0; i < 4; ++i)
        {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];

            // ����������ڵ�ͼ��Χ����δ���ʹ��Ҳ����ϰ����������
            if (isValid(api, newX, newY) && !visited[newX][newY] && (grid[newX][newY] == 0))
            {
                visited[newX][newY] = true;
                parent[newX][newY] = {current.x, current.y};
                q.push({newX, newY});
                //std::string temp = "(" + std::to_string(newX) + "," + std::to_string(newY) + ")\n";
                //api.Print(temp);
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
            //std::string temp = "Call back : (" + std::to_string(current.x) + "," + std::to_string(current.y) + ")\n";
            //api.Print(temp);
        }
        Path.push_back(start);
        reverse(Path.begin(), Path.end());
        api.Print("The Path Have Already Got!");
    }
    else
    {
        api.Print("No path found! ");
    }
    //std::string path_size = std::to_string(Path.size());
    //api.Print(path_size);
    return Path;
}
bool Path_Release(std::vector<Point> Path, IShipAPI& api, int count)
{
    int Path_size = Path.size();
    if (count >= Path_size - 1)
    {
        return true;
    }
    int delta_x = Path[count + 1].x - Path[count].x;
    int delta_y = Path[count + 1].y - Path[count].y;
    bool judge = false;

    if (delta_x == -1)
    {
        api.MoveUp(1000 / SPEED_CIVIL_MS);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else if (delta_x == 1)
    {
        api.MoveDown(1000);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else if (delta_y == 1)
    {
        api.MoveRight(1000);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else if (delta_y == -1)
    {
        api.MoveLeft(1000);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return false;
}

void Get_Resource(IShipAPI& api)
{   // ����ȫ�Զ���Դ���ɣ�ֱ���ѱ�����ͼ��������Դ������Ϊֹ
    // ����֮������xΪ�������������ܺ�ʱ�ϳ�������sort����Ҳ������
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
    for (int i = 0; i < size; i++)
    {
        int x = resource_vec[i].x;
        int y = resource_vec[i].y;
        if ((cellx <= 25 && x <= 25) || (cellx >= 27 && x >= 27))
        {
            GoPlace_Loop(api, resource_vec[i].x_4p, resource_vec[i].y_4p);
            int state = api.GetResourceState(x, y);
            int count = 0;
            while (state > 0)
            {   // ֻҪ����ʣ����Դ�Ϳ���
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

void Build_ALL(IShipAPI& api, THUAI7::ConstructionType type)
{
    auto selfinfo = api.GetSelfInfo();
    int gridx = selfinfo->x;
    int gridy = selfinfo->y;
    int cellx = api.GridToCell(gridx);
    int celly = api.GridToCell(gridy);

    int IntendedHp = 8000;
    int size = construction_vec.size();
    if (size == 0)
    {
        api.Print("NO VALID CONSTRUCTION POINT!");
        return;
    }
    if (type == THUAI7::ConstructionType::Factory)
    {
        IntendedHp = 8000;
    }
    else if (type == THUAI7::ConstructionType::Fort)
    {
        IntendedHp = 12000;
    }
    else if (type == THUAI7::ConstructionType::Community)
    {
        IntendedHp = 6000;
    }
    for (int i = 0; i < size; i++)
    {
        if ((cellx <= 25 && construction_vec[i].x <= 25) || (cellx >= 27 && construction_vec[i].x >= 27))
        {
            GoPlace_Loop(api, construction_vec[i].x_4c, construction_vec[i].y_4c);
            int Hp = api.GetConstructionHp(construction_vec[i].x, construction_vec[i].y);
            if (Hp < IntendedHp)
            {
                int count = 0;
                while (Hp < IntendedHp)
                {
                    api.Construct(type);  
                    count++;
                    if (count % 10 == 0)
                    {
                        api.Wait();
                        Hp = api.GetConstructionHp(construction_vec[i].x, construction_vec[i].y);
                        count = 0;
                    }
                } 
            }
        }
    }
}

void Construct_Module(ITeamAPI& api, int shipno, int limit, int type)
{
    auto constructtype = api.GetShips()[shipno]->constructorType;
    int energy = api.GetEnergy();
    if (energy < limit)
    {
        return;
    }
    switch (type)
    {
        case 2 :
            if (constructtype == THUAI7::ConstructorType::Constructor1)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleConstructor2);
            }

            break;

        case 3 :
            if (constructtype != THUAI7::ConstructorType::Constructor3)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleConstructor3);
            }

            break;
    }
    return;
}

void Produce_Module(ITeamAPI& api, int shipno, int limit, int type)
{
    if (api.GetShips()[shipno-1] == nullptr)
    {
        return;
    }
    auto producetype = api.GetShips()[shipno-1]->producerType;
    int energy = api.GetEnergy();
    switch (type)
    {
        case 2:
            if (producetype == THUAI7::ProducerType::Producer1 && energy > limit)
            {   
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleProducer2);
            }
        case 3:
            if (producetype != THUAI7::ProducerType::Producer3 && energy > limit)
            {
                api.InstallModule(shipno, THUAI7::ModuleType::ModuleProducer3);
            }
    }
    return;
}

void Build_Ship(ITeamAPI& api, int shipno, int birthdes)
{
    auto selfinfo = api.GetSelfInfo();
    int energy = selfinfo->energy;
    auto ships = api.GetShips();
    int size = ships.size();

    if (shipno <= size && ships[shipno - 1]->hp > 0)
    {
        api.Print("Ship Already Existed! ");
        return;
    }

    if (ShipTypeDict[shipno-1] == THUAI7::ShipType::CivilianShip)
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
    else if (ShipTypeDict[shipno-1] == THUAI7::ShipType::MilitaryShip)
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
    else if (ShipTypeDict[shipno-1] == THUAI7::ShipType::FlagShip)
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