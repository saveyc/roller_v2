#include "data.h"

//区域配置信息
sData_zone_config     zoneConfig;
sData_zone_config     zonetmp;

//滚筒配置参数
sData_belt_msg        beltMoudlepara;
//滚筒模块实时参数
sbelt_Moudle_state    beltMoudlestate;


sData_pkg_node        pgkNodeDateItem[MAX_PKG_NUM];
struct xLIST_ITEM     pkgNodeListItem[MAX_PKG_NUM];

void data_msg_init(void)
{
	u16 i = 0;
	u16 j = 0;


	vListInitialise(&pkg_list);

	for (i = 0; i < BELT_ZONE_NUM; i++)
	{
		zoneConfig.zoneNode[i].zoneIndex = 0xFFFF;    //默认不存在
		zoneConfig.zoneNode[i].ctrlIndex = 0xFFFF;    //默认不存在
	}

	for (i = 0; i < BELT_ZONE_NUM; i++) {
		beltMoudlestate.state[i].ctrlindex = i + 2;    
		for (j = 0; j < ZONE_NUM; j++) {
			beltMoudlestate.state[i].node[j].zoneIndex = 0xFFFF;  //默认不配置区域
		}
	}

	zoneConfig.confid = 0;
	zoneConfig.flashcnt = 0;
	zoneConfig.pkgcal = 0;


	zonetmp.confid = 0;
	zonetmp.flashcnt = 0;
	zonetmp.pkgcal = 0;
}


void data_pkg_list_init(void)
{
	u16 i = 0;
	sData_pkg_node* pkgnode = NULL;
	for (i = 0; i < MAX_PKG_NUM; i++) {
		pkgnode = &(pgkNodeDateItem[i]);
		pkgnode->index = &(pkgNodeListItem[i]);
		pkgnode->index->pvOwner = &(pgkNodeDateItem[i]);
		pkgnode->index->xItemValue = i;
		vListInitialiseItem(pkgnode->index);
	}
}

void data_addto_pkg_list(sData_pkg_node x)
{
	u16 i = 0;
	u16 j = 0;

	sData_pkg_node* pkgnode = NULL;
	for (i = 0; i < MAX_PKG_NUM; i++) {
		pkgnode = &(pgkNodeDateItem[i]);
		pkgnode->index = &(pkgNodeListItem[i]);
		pkgnode->index->pvOwner = &(pgkNodeDateItem[i]);
		pkgnode->index->xItemValue = i;
		if (pkgnode->index->pvContainer == NULL) {
			break;
		}
	}

	if (i == MAX_PKG_NUM) {
		return;
	}

	pgkNodeDateItem[i].pkgId = x.pkgId;
	pgkNodeDateItem[i].totalzoneNum = x.totalzoneNum;
	for (j = 0; j < pgkNodeDateItem[i].totalzoneNum; j++) {
		pgkNodeDateItem[i].zoneindex[j] = x.zoneindex[j];
	}

	pgkNodeDateItem[i].allowState = TRANS_ALLOW;
	pgkNodeDateItem[i].curZoneCnt = 0;
	pgkNodeDateItem[i].nextZoneCnt = 0;

	pgkNodeDateItem[i].curZonenum = 0;
	pgkNodeDateItem[i].lastZonenum = 0xFFFF;
	pgkNodeDateItem[i].sendcmdcnt = 0;

	vListInsert(&pkg_list, pgkNodeDateItem[i].index);
}

sData_zone_node* data_find_zone_moudlestate(u16 index)
{
	u16 i = 0;

	for (i = 0; i < BELT_ZONE_NUM; i++) {
		if (zoneConfig.zoneNode[i].zoneIndex == index) {
			return (sData_zone_node*)(&(zoneConfig.zoneNode[i]));
		}
	}
	return NULL;
}

sData_RealtimeState* data_find_ctrl_status(u16 ctrlindex, u16 moudleindex)
{
	if (ctrlindex < 2) {
		return NULL;
	}
	if (ctrlindex >= BELT_ZONE_NUM) {
		return NULL;
	}
	if (moudleindex == ZONE_TYPE_ONE) {
		return (sData_RealtimeState*)(&(beltMoudlestate.state[ctrlindex - 2].node[0]));
	}
	if (moudleindex == ZONE_TYPE_TWO) {
		return (sData_RealtimeState*)(&(beltMoudlestate.state[ctrlindex - 2].node[1]));
	}
	if (moudleindex == ZONE_TYPE_RISE) {
		return (sData_RealtimeState*)(&(beltMoudlestate.state[ctrlindex - 2].node[2]));
	}

	return NULL;
}

//计算该模块 相邻模块相对于该模块的方向
u8 data_find_nearzone_direction( sData_zone_node node, u16 nextzone)
{
    
	if (nextzone == FINAL_CAR) {
		return DIR_NONE;
	}
	if (node.front_zone == nextzone) {
		return FRONT_DIR;
	}
	if (node.rear_zone == nextzone) {
		return BEHIND_DIR;
	}
	if (node.left_zone == nextzone) {
		return LEFT_DIR;
	}
	if (node.right_zone == nextzone) {
		return RIGHT_DIR;
	}
	
	return DIR_NONE;
}

u8 data_config_moudle_rundir(u8 predir, u8 nextdir)
{
	//从相对于该模块后面的模块 输送到其它的模块
	if ((predir == BEHIND_DIR) && (nextdir == FRONT_DIR)) {
		return RUN_BACK_TOAHEAD;
	}
	if ((predir == BEHIND_DIR) && (nextdir == DIR_NONE)) {
		return RUN_BACK_TOAHEAD;
	}
	if ((predir == BEHIND_DIR) && (nextdir == LEFT_DIR)) {
		return RUN_BACK_TOLEFT;
	}
	if ((predir == BEHIND_DIR) && (nextdir == RIGHT_DIR)) {
		return RUN_BACK_TORIGHT;
	}

	//从相对于该模块前面的模块 输送到其它的模块
	if ((predir == FRONT_DIR) && (nextdir == BEHIND_DIR)) {
		return RUN_AHEAD_TOBACK;
	}
	if ((predir == FRONT_DIR) && (nextdir == DIR_NONE)) {
		return RUN_AHEAD_TOBACK;
	}
	if ((predir == FRONT_DIR) && (nextdir == LEFT_DIR)) {
		return RUN_AHEAD_TOLEFT;
	}
	if ((predir == FRONT_DIR) && (nextdir == RIGHT_DIR)) {
		return RUN_AHEAD_TORIGHT;
	}

	//从相对于该模块左边的模块 输送到其它的模块
	if ((predir == LEFT_DIR) && (nextdir == BEHIND_DIR)) {
		return RUN_LEFT_TOBACK;
	}
	if ((predir == LEFT_DIR) && (nextdir == FRONT_DIR)) {
		return RUN_LEFT_TOAHEAD;
	}
	if ((predir == LEFT_DIR) && (nextdir == DIR_NONE)) {
		return RUN_LEFT_TORIGHT;
	}
	if ((predir == LEFT_DIR) && (nextdir == RIGHT_DIR)) {
		return RUN_LEFT_TORIGHT;
	}

	//从相对于该模块右边的模块 输送到其它的模块
	if ((predir == RIGHT_DIR) && (nextdir == BEHIND_DIR)) {
		return RUN_RIGHT_TOBACK;
	}
	if ((predir == RIGHT_DIR) && (nextdir == FRONT_DIR)) {
		return RUN_RIGHT_TOAHEAD;
	}
	if ((predir == RIGHT_DIR) && (nextdir == DIR_NONE)) {
		return RUN_RIGHT_TOLEFT;
	}
	if ((predir == RIGHT_DIR) && (nextdir == LEFT_DIR)) {
		return RUN_RIGHT_TOLEFT;
	}

	//该模块前面没有模块 输送到其它的模块
	if ((predir == DIR_NONE) && (nextdir == BEHIND_DIR)) {
		return RUN_AHEAD_TOBACK;
	}
	if ((predir == DIR_NONE) && (nextdir == FRONT_DIR)) {
		return RUN_BACK_TOAHEAD;
	}
	if ((predir == DIR_NONE) && (nextdir == RIGHT_DIR)) {
		return RUN_LEFT_TORIGHT;
	}
	if ((predir == DIR_NONE) && (nextdir == LEFT_DIR)) {
		return RUN_RIGHT_TOLEFT;
	}

    return RUN_DIRDEFAULT;
}





