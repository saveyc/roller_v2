#include "main.h"
#include "logic_ctrl.h"

u16 cansend_framecnt_one[BELT_ZONE_NUM] = {0};
u16 cansend_framecnt_two[BELT_ZONE_NUM] = {0};
u16 cansend_framecnt_rise[BELT_ZONE_NUM] = {0};

#define MODULENODENUM   50
sModule_node   moduleNode[50];

void logicModuleNodeInit(void)
{
	u16 i = 0;
	for (i = 0; i < MODULENODENUM; i++) {
		moduleNode[i].value = INVALUE;
	}
}

void logicAddModuleNodeQueue(sModule_node x)
{
	u16 i = 0;
	for (i = 0; i < MODULENODENUM; i++) {
		if (moduleNode[i].value == INVALUE) {
			moduleNode[i] = x;
			moduleNode[i].value = VALUE;
		}
	}
}

void logicDealWithModuleNodeQueue(void)
{
	u16 i = 0;
	sMoudle_cmd  moudletmp;

	for (i = 0; i < MODULENODENUM; i++) {
		if (moduleNode[i].value == VALUE) {
			if (moduleNode[i].chutDown > 0) {
				moduleNode[i].chutDown--;
			}
                        if (moduleNode[i].chutDown == 0) {
                            moduleNode[i].value = INVALUE;
                    
                            if (moduleNode[i].cmd.moudle == ZONE_TYPE_ONE) {
                                    cansend_framecnt_one[moduleNode[i].ctrlIndex - 2]++;
                                    moudletmp.cmd = moduleNode[i].cmd.cmd;
                                    moudletmp.moudle = moduleNode[i].cmd.moudle;
                                    vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[moduleNode[i].ctrlIndex - 2], moduleNode[i].ctrlIndex);
                            }
                            if (moduleNode[i].cmd.moudle == ZONE_TYPE_TWO) {
                                    cansend_framecnt_two[moduleNode[i].ctrlIndex - 2]++;
                                    moudletmp.cmd = moduleNode[i].cmd.cmd;
                                    moudletmp.moudle = moduleNode[i].cmd.moudle;
                                    vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[moduleNode[i].ctrlIndex - 2], moduleNode[i].ctrlIndex);
                            }
                            if (moduleNode[i].cmd.moudle == ZONE_TYPE_RISE) {
                                    cansend_framecnt_rise[moduleNode[i].ctrlIndex - 2]++;
                                    moudletmp.cmd = moduleNode[i].cmd.cmd;
                                    moudletmp.moudle = moduleNode[i].cmd.moudle;
                                    vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[moduleNode[i].ctrlIndex - 2], moduleNode[i].ctrlIndex);
                            }
                    }
		}
	}
}



// 包裹传输过程逻辑 1ms执行1次
void logic_pkg_trans_process(void)
{
	List_t* q = NULL;
	u16 itemvalue = 0;
	sData_pkg_node* pkgnode = NULL;
	sData_zone_node* prenode = NULL;
	sData_zone_node* curnode = NULL;
	sData_zone_node* nextnode = NULL;
	sData_RealtimeState* prestatenode = NULL;
	sData_RealtimeState* curstatenode = NULL;
	sData_RealtimeState* nextstatenode = NULL;
	sMoudle_cmd  moudlecmdcur;
	sMoudle_cmd  moudlecmdnext;
	sMoudle_cmd  moudletmp;
	u8 predir = DIR_NONE;
	u8 nextdir = DIR_NONE;
	u16 nextpkgstat = 0;
	sPkgResultNode  resultnode;
	sModule_node moduleprenode;


	moudlecmdcur.cmd = RUN_DEFAULT;
	moudlecmdnext.cmd = RUN_DEFAULT;
	moudletmp.cmd = RUN_DEFAULT;


	q = &pkg_list;
	// 
	itemvalue = q->xListEnd.xItemValue;

 	for (q->pxIndex = (ListItem_t*)(q->xListEnd.pxNext); q->pxIndex->xItemValue != itemvalue; q->pxIndex = q->pxIndex->pxNext)
	{
		pkgnode = q->pxIndex->pvOwner;
		if (pkgnode->allowState == TRANS_ALLOW) {      
			if (pkgnode->curZonenum <= (pkgnode->totalzoneNum - 1)) {
				pkgnode->sendcmdcnt++;
				pkgnode->curZoneCnt++;
				pkgnode->nextZoneCnt++;
				if (pkgnode->preZoneChutdown > 0) {
					pkgnode->preZoneChutdown--;
				}
				//更新前一个区域的信息
				if (pkgnode->curZonenum != 0) {
					prenode = data_find_zone_moudlestate(pkgnode->zoneindex[pkgnode->curZonenum - 1]);
					prestatenode = data_find_ctrl_status(prenode->ctrlIndex, prenode->beltMoudleIndex);
					//if (prestatenode->zonePkg == pkgnode->pkgId) {
					//	prestatenode->zonePkg = 0;

					//	prestatenode->transsuccess = INVALUE;
					//	prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
					//	prestatenode->zoneState |= (1 << 5);         //置为无货状态

					//	prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
					//}
					//if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
					//	prestatenode->zoneNextpkg = 0;
					//}
				}
				else {
					prenode = NULL;
				}

			    //更新当前区域信息
				pkgnode->curZoneIndex = pkgnode->zoneindex[pkgnode->curZonenum];
				curnode = data_find_zone_moudlestate(pkgnode->curZoneIndex);
				curstatenode = data_find_ctrl_status(curnode->ctrlIndex,curnode->beltMoudleIndex);



				//更新下一区域信息
				if ((pkgnode->curZonenum) >= (pkgnode->totalzoneNum - 1)) {    //当前区域已经是最后一个区域
					pkgnode->nextZoneIndex = FINAL_CAR;
					nextnode = NULL;
					nextstatenode = NULL;
				}
				else {
					pkgnode->nextZoneIndex = pkgnode->zoneindex[pkgnode->curZonenum + 1];
					nextnode = data_find_zone_moudlestate(pkgnode->nextZoneIndex);
					nextstatenode = data_find_ctrl_status(nextnode->ctrlIndex, nextnode->beltMoudleIndex);
				}

				//更新下一个区域的下一个区域
				if ((pkgnode->curZonenum) >= (pkgnode->totalzoneNum - 2)) {
					pkgnode->theThirdZone = FINAL_CAR;
				}
				else {
					pkgnode->theThirdZone = pkgnode->zoneindex[pkgnode->curZonenum + 2];
				}

				//计算当前区域的传输方向
				if (pkgnode->curZonenum == 0) {
					predir = data_find_nearzone_direction(*curnode, FINAL_CAR);
				}
				else {
			        predir = data_find_nearzone_direction(*curnode, pkgnode->zoneindex[pkgnode->curZonenum - 1]);
				}
				nextdir = data_find_nearzone_direction(*curnode, pkgnode->nextZoneIndex);
				pkgnode->curZonedir = data_config_moudle_rundir(predir, nextdir);
				
				//计算下一个欲前往区域的传输方向
				if (pkgnode->nextZoneIndex != FINAL_CAR) {
					predir = data_find_nearzone_direction(*nextnode, pkgnode->curZoneIndex);
					nextdir = data_find_nearzone_direction(*nextnode, pkgnode->theThirdZone);
					pkgnode->nextZonedir = data_config_moudle_rundir(predir, nextdir);
				}

#if 0
				//判断下一区域的是否有包裹   单纯依据光电判断
				if (nextstatenode != NULL) {
					if (pkgnode->nextZonedir == RUN_AHEAD_TOBACK) {
						 nextpkgstat = ((((nextstatenode->zoneState >> 1) & 0x1) == 1)|| (((nextstatenode->zoneState >> 3) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
					if (pkgnode->nextZonedir == RUN_AHEAD_TOBACK) {
						nextpkgstat = (((nextstatenode->zoneState >> 1) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
					if (pkgnode->nextZonedir == RUN_RIGHT_TOBACK) {
						nextpkgstat = (((nextstatenode->zoneState >> 3) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}


					if (pkgnode->nextZonedir == RUN_BACK_TOAHEAD) {
						nextpkgstat = ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
						
					}
					if (pkgnode->nextZonedir == RUN_LEFT_TOAHEAD) {
						nextpkgstat = (((nextstatenode->zoneState >> 4) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}

					}
					if (pkgnode->nextZonedir == RUN_RIGHT_TOAHEAD) {
						nextpkgstat = (((nextstatenode->zoneState >> 2) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}

					}


					if (pkgnode->nextZonedir == RUN_RIGHT_TOLEFT) {
						nextpkgstat = ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 3) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}						
					}
					if (pkgnode->nextZonedir == RUN_AHEAD_TOLEFT) {
						nextpkgstat = (((nextstatenode->zoneState >> 3) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
					if (pkgnode->nextZonedir == RUN_BACK_TOLEFT) {
						nextpkgstat = (((nextstatenode->zoneState >> 2) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}

					if (pkgnode->nextZonedir == RUN_LEFT_TORIGHT) {
						nextpkgstat = (((nextstatenode->zoneState >> 1) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
					if (pkgnode->nextZonedir == RUN_AHEAD_TORIGHT) {
						nextpkgstat = (((nextstatenode->zoneState >> 1) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
					if (pkgnode->nextZonedir == RUN_BACK_TORIGHT) {
						nextpkgstat = (((nextstatenode->zoneState >> 4) & 0x1) == 1);
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}
				}
#else
				//判断下一区域是否有包裹   依据模块状态信息判断  已经有包裹或者模块已经被占据
				if (nextstatenode != NULL) {
					nextpkgstat = (((nextstatenode->zonePkg != 0) ? 1 : 0) || ((nextstatenode->zoneNextpkg != 0) && 
						          (nextstatenode->zoneNextpkg != pkgnode->pkgId)) );
					if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
						pkgnode->nextZonecurpkgstat = nextpkgstat;
						pkgnode->nextZonepkgstatchange = VALUE;
						pkgnode->nextZonechangenum++;
					}
				}
#endif
				//上一个区域状态更新 检测到没有光电触发 倒计时停止模块
				if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId) && (pkgnode->preZoneChutdown == 0)) {
					if (((pkgnode->lastZoneDir == RUN_AHEAD_TOBACK) && (((prestatenode->zoneState >> 1) & 0x1) == 0)) ||
							((pkgnode->lastZoneDir == RUN_LEFT_TOBACK) && (((prestatenode->zoneState >> 1) & 0x1) == 0)) ||
							((pkgnode->lastZoneDir == RUN_RIGHT_TOBACK) && (((prestatenode->zoneState >> 3) & 0x1) == 0))) {
						//给前一段模块发停止指令
						if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
							prestatenode->zonePkg = 0;
							if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
								prestatenode->zoneNextpkg = 0;
							}

							moudletmp.moudle = prenode->beltMoudleIndex;
							moudletmp.cmd = STOP_CMD;

							moduleprenode.cmd = moudletmp;
							moduleprenode.chutDown = 0;
							moduleprenode.ctrlIndex = prenode->ctrlIndex;
							moduleprenode.value = VALUE;
							logicAddModuleNodeQueue(moduleprenode);

							//更新前一段的状态
							prestatenode->transsuccess = INVALUE;
							prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
							prestatenode->zoneState |= (1 << 5);         //置为无货状态

							prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态


						}
					}

					if (((pkgnode->lastZoneDir == RUN_BACK_TOAHEAD) && ((((prestatenode->zoneState >> 2) & 0x1) == 0))) ||
						((pkgnode->lastZoneDir == RUN_LEFT_TOAHEAD) && (((prestatenode->zoneState >> 4) & 0x1) == 0)) ||
						((pkgnode->lastZoneDir == RUN_RIGHT_TOAHEAD) && (((prestatenode->zoneState >> 2) & 0x1) == 0))) {
						//给前一段模块发停止指令
						if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
							prestatenode->zonePkg = 0;

							moudletmp.moudle = prenode->beltMoudleIndex;
							moudletmp.cmd = STOP_CMD;

							moduleprenode.cmd = moudletmp;
							moduleprenode.chutDown = 0;
							moduleprenode.ctrlIndex = prenode->ctrlIndex;
							moduleprenode.value = VALUE;
							logicAddModuleNodeQueue(moduleprenode);

							//更新前一段的状态
							prestatenode->transsuccess = INVALUE;
							prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
							prestatenode->zoneState |= (1 << 5);         //置为无货状态

							prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态

							if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
								prestatenode->zoneNextpkg = 0;
							}
						}
					}


					if (((pkgnode->lastZoneDir == RUN_RIGHT_TOLEFT) && ((((prestatenode->zoneState >> 2) & 0x1) == 0))) ||
						((pkgnode->lastZoneDir == RUN_AHEAD_TOLEFT) && (((prestatenode->zoneState >> 3) & 0x1) == 0)) ||
						((pkgnode->lastZoneDir == RUN_BACK_TOLEFT) && (((prestatenode->zoneState >> 2) & 0x1) == 0))) {
						//给前一段模块发停止指令
						if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
							prestatenode->zonePkg = 0;


							moudletmp.moudle = prenode->beltMoudleIndex;
							moudletmp.cmd = STOP_CMD;

							moduleprenode.cmd = moudletmp;
							moduleprenode.chutDown = 0;
							moduleprenode.ctrlIndex = prenode->ctrlIndex;
							moduleprenode.value = VALUE;
							logicAddModuleNodeQueue(moduleprenode);

							//更新前一段的状态
							prestatenode->transsuccess = INVALUE;
							prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
							prestatenode->zoneState |= (1 << 5);         //置为无货状态

							prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态

							if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
								prestatenode->zoneNextpkg = 0;
							}
						}

					}

					if (((pkgnode->lastZoneDir == RUN_LEFT_TORIGHT) && ((((prestatenode->zoneState >> 1) & 0x1) == 0))) ||
						((pkgnode->lastZoneDir == RUN_AHEAD_TORIGHT) && (((prestatenode->zoneState >> 1) & 0x1) == 0)) ||
						((pkgnode->lastZoneDir == RUN_BACK_TORIGHT) && (((prestatenode->zoneState >> 4) & 0x1) == 0))) {
						//给前一段模块发停止指令
						if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
							prestatenode->zonePkg = 0;


							moudletmp.moudle = prenode->beltMoudleIndex;
							moudletmp.cmd = STOP_CMD;

							moduleprenode.cmd = moudletmp;
							moduleprenode.chutDown = 0;
							moduleprenode.ctrlIndex = prenode->ctrlIndex;
							moduleprenode.value = VALUE;
							logicAddModuleNodeQueue(moduleprenode);

							//更新前一段的状态
							prestatenode->transsuccess = INVALUE;
							prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
							prestatenode->zoneState |= (1 << 5);         //置为无货状态

							prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态

							if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
								prestatenode->zoneNextpkg = 0;
							}
						}
					}
				}

				//当前区域状态更新  
				//已经到达最后一段区域  (认为最后一段非顶升模块)
				if ((pkgnode->curZonenum) == (pkgnode->totalzoneNum - 1)) {
					curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
					curstatenode->zoneState |= (0x2 << 5);       //置为有货状态

					curstatenode->zoneAlarm &= ~(0x1 << 9);      //清除当前段接货超时报警

					//curstatenode->zoneState &= 0xFE5F;
					curstatenode->zonePkg = pkgnode->pkgId;

					if (curstatenode->zoneNextpkg == pkgnode->pkgId) {
						curstatenode->zoneNextpkg = 0;
					}

					if (((((curstatenode->zoneState >> 2) & 0x1) == 1) && (pkgnode->curZonedir == RUN_BACK_TOAHEAD))
						|| ((((curstatenode->zoneState >> 1) & 0x1) == 1) && (pkgnode->curZonedir == RUN_AHEAD_TOBACK))) {
						     
						pkgnode->allowState = TRANS_SUCCESS;
						//上传运输结果
						resultnode.pkgid = pkgnode->pkgId;
						resultnode.result = TRANS_SUCCESS;
						DataaddPkgQueue(resultnode);
						pkgnode->curZoneCnt = 0;
						pkgnode->nextZoneCnt = 0;
						if (curstatenode->transsuccess == INVALUE) {
							// 给当前段模块重发停止指令
							if (curnode->beltMoudleIndex == ZONE_TYPE_ONE) {
								cansend_framecnt_one[curnode->ctrlIndex - 2]++;
								moudletmp.cmd = STOP_CMD;
								moudletmp.moudle = ZONE_TYPE_ONE;
								vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[curnode->ctrlIndex - 2], curnode->ctrlIndex);

							}
							if (curnode->beltMoudleIndex == ZONE_TYPE_TWO) {
								cansend_framecnt_two[curnode->ctrlIndex - 2]++;
								moudletmp.cmd = STOP_CMD;
								moudletmp.moudle = ZONE_TYPE_TWO;
								vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[curnode->ctrlIndex - 2], curnode->ctrlIndex);                                                                                                                                                                                                                                                  
							}
							//给前一段模块发停止指令
							if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
								prestatenode->zonePkg = 0;
								if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
									cansend_framecnt_one[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_ONE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
									cansend_framecnt_two[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_TWO;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
									cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_RISE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}

								//更新前一段的状态
								prestatenode->transsuccess = INVALUE;
								prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
								prestatenode->zoneState |= (1 << 5);         //置为无货状态

								prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态

								if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
									prestatenode->zoneNextpkg = 0;
								}
							}
							//应该检测到光电变化再清空模块货物信息 **
							//curstatenode->zonePkg = 0;
							//curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
							curstatenode->transsuccess = VALUE;

						}
						curstatenode->transsuccess = VALUE;
						uxListRemove(pkgnode->index);
						continue;
//						return;
					}
				}
				else {
					curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
					curstatenode->zoneState |= (0x2 << 5);         //置为有货状态

					curstatenode->zoneAlarm &= ~(0x1 << 9);      //清除接货超时报警
					curstatenode->zonePkg = pkgnode->pkgId;

					//在本模块时试图去占据下一段的模块的使用权 解除当前模块的欲使用权
					if (curstatenode->zoneNextpkg == pkgnode->pkgId) {
						curstatenode->zoneNextpkg = 0;
					}
					if (nextstatenode->zoneNextpkg == 0) {
						nextstatenode->zoneNextpkg = pkgnode->pkgId;
					}

					if(((pkgnode->curZonedir == RUN_AHEAD_TOBACK) && ((((curstatenode->zoneState >> 1) & 0x1) == 1)|| (((curstatenode->zoneState >> 3) & 0x1) == 1))) ||
						((pkgnode->curZonedir == RUN_LEFT_TOBACK) && (((curstatenode->zoneState >> 1) & 0x1) == 1)) ||
						((pkgnode->curZonedir == RUN_RIGHT_TOBACK) && (((curstatenode->zoneState >> 3) & 0x1) == 1))){

					//if (((pkgnode->curZonedir == RUN_AHEAD_TOBACK) || (pkgnode->curZonedir == RUN_LEFT_TOBACK)
					//	|| (pkgnode->curZonedir == RUN_RIGHT_TOBACK)) && ((((curstatenode->zoneState >> 1) & 0x1) == 1) 
					//		|| (((curstatenode->zoneState >> 3) & 0x1) == 1))) {
						//curstatenode->zoneState &= 0xFE9F;
						curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
						curstatenode->zoneState |= (0x4 << 5);       //置为正在出货
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
								prestatenode->zonePkg = 0;
								if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
									cansend_framecnt_one[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_ONE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
									cansend_framecnt_two[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_TWO;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
									cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_RISE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								//更新前一段的状态

								prestatenode->transsuccess = INVALUE;
								prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
								prestatenode->zoneState |= (1 << 5);         //置为无货状态

								prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
								if (prestatenode->zoneNextpkg == pkgnode->pkgId) {
									prestatenode->zoneNextpkg = 0;
								}
							}

							//当前段应该倒计时停止
							pkgnode->curZoneArrive = VALUE;
						}
					}

					if (((pkgnode->curZonedir == RUN_BACK_TOAHEAD) && ((((curstatenode->zoneState >> 2) & 0x1) == 1) || (((curstatenode->zoneState >> 4) & 0x1) == 1))) ||
						((pkgnode->curZonedir == RUN_LEFT_TOAHEAD) && (((curstatenode->zoneState >> 4) & 0x1) == 1)) ||
						((pkgnode->curZonedir == RUN_RIGHT_TOAHEAD) && (((curstatenode->zoneState >> 2) & 0x1) == 1))) {
					//if (((pkgnode->curZonedir == RUN_BACK_TOAHEAD) || (pkgnode->curZonedir == RUN_LEFT_TOAHEAD)
					//	|| (pkgnode->curZonedir == RUN_RIGHT_TOAHEAD)) && ((((curstatenode->zoneState >> 2) & 0x1) == 1) 
					//		|| (((curstatenode->zoneState >> 4) & 0x1) == 1))) {
						//curstatenode->zoneState &= 0xFE9F;
						curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
						curstatenode->zoneState |= (0x4 << 5);       //置为正在出货
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
								prestatenode->zonePkg = 0;
								if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
									cansend_framecnt_one[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_ONE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
									cansend_framecnt_two[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_TWO;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
									cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_RISE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}

								//更新前一段的状态

								prestatenode->transsuccess = INVALUE;
								prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
								prestatenode->zoneState |= (1 << 5);         //置为无货状态

								prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
							}

							//当前段应该倒计时停止
							pkgnode->curZoneArrive = VALUE;
						}
					}


					if (((pkgnode->curZonedir == RUN_RIGHT_TOLEFT) && ((((curstatenode->zoneState >> 2) & 0x1) == 1) || (((curstatenode->zoneState >> 3) & 0x1) == 1))) ||
						((pkgnode->curZonedir == RUN_AHEAD_TOLEFT) && (((curstatenode->zoneState >> 3) & 0x1) == 1)) ||
						((pkgnode->curZonedir == RUN_BACK_TOLEFT) && (((curstatenode->zoneState >> 2) & 0x1) == 1))) {

					//if (((pkgnode->curZonedir == RUN_RIGHT_TOLEFT) || (pkgnode->curZonedir == RUN_AHEAD_TOLEFT)
					//	|| (pkgnode->curZonedir == RUN_BACK_TOLEFT)) && ((((curstatenode->zoneState >> 2) & 0x1) == 1)
					//		|| (((curstatenode->zoneState >> 3) & 0x1) == 1))) {
						//curstatenode->zoneState &= 0xFE9F;
						curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
						curstatenode->zoneState |= (0x4 << 5);       //置为正在出货
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
								prestatenode->zonePkg = 0;
								if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
									cansend_framecnt_one[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_ONE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
									cansend_framecnt_two[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_TWO;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
									cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_RISE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}

								//更新前一段的状态

								prestatenode->transsuccess = INVALUE;
								prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
								prestatenode->zoneState |= (1 << 5);         //置为无货状态

								prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
							}

							//当前段应该倒计时停止
							pkgnode->curZoneArrive = VALUE;
						}
					}

					if (((pkgnode->curZonedir == RUN_LEFT_TORIGHT) && ((((curstatenode->zoneState >> 1) & 0x1) == 1) || (((curstatenode->zoneState >> 4) & 0x1) == 1))) ||
						((pkgnode->curZonedir == RUN_AHEAD_TORIGHT) && (((curstatenode->zoneState >> 1) & 0x1) == 1)) ||
						((pkgnode->curZonedir == RUN_BACK_TORIGHT) && (((curstatenode->zoneState >> 4) & 0x1) == 1))) {
					//if (((pkgnode->curZonedir == RUN_LEFT_TORIGHT) || (pkgnode->curZonedir == RUN_AHEAD_TORIGHT)
					//	|| (pkgnode->curZonedir == RUN_BACK_TORIGHT)) && ((((curstatenode->zoneState >> 1) & 0x1) == 1)
					//		|| (((curstatenode->zoneState >> 4) & 0x1) == 1))) {
						//curstatenode->zoneState &= 0xFE9F;
						curstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
						curstatenode->zoneState |= (0x4 << 5);       //置为正在出货
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {
								prestatenode->zonePkg = 0;
								if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
									cansend_framecnt_one[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_ONE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
									cansend_framecnt_two[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_TWO;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}
								if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
									cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
									moudletmp.cmd = STOP_CMD;
									moudletmp.moudle = ZONE_TYPE_RISE;
									vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
								}

								//更新前一段的状态

								prestatenode->transsuccess = INVALUE;
								prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
								prestatenode->zoneState |= (1 << 5);         //置为无货状态

								prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
							}

							//当前段应该倒计时停止
							pkgnode->curZoneArrive = VALUE;
						}
					}
				
                }

				//更新下一个区域的状态 并且判断切换区域
				if (pkgnode->nextZoneIndex != FINAL_CAR) {
					//nextstatenode->transsuccess = INVALUE;
//					nextstatenode->zonePkg = 0;
					//nextstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
					//nextstatenode->zoneState |= (0x8 << 5);       //置为等待接货状态
                                         
                    // 下一个区域光电没检测到货物 且没有货                   
                    // if ((pkgnode->nextZonecurpkgstat == 0) && (nextstatenode->zonePkg == 0)) {
					// 下一个区域没有货 不根据光电判断 且没有货准备进入
					if ((nextstatenode->zonePkg == 0) && ((nextstatenode->zoneNextpkg == 0) ||
						(nextstatenode->zoneNextpkg == pkgnode->pkgId))) {

						nextstatenode->zoneState &= ~(0xF << 5);      //清除货物状态
						nextstatenode->zoneState |= (0x8 << 5);       //置为等待接货状态
						//判断停止前一个区域运输
                        if ((((nextstatenode->zoneState >> 1) & 0x1) == 1) || (((nextstatenode->zoneState >> 2) & 0x1) == 1)
                                || (((nextstatenode->zoneState >> 3) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1)) {

                                if ((prenode != NULL) && (prestatenode->zonePkg == pkgnode->pkgId)) {	
									    prestatenode->zonePkg = 0;
                                        //停止前一个区域传输
                                        if (prenode->beltMoudleIndex == ZONE_TYPE_ONE) {
                                                cansend_framecnt_one[prenode->ctrlIndex - 2]++;
                                                moudletmp.cmd = STOP_CMD;
                                                moudletmp.moudle = ZONE_TYPE_ONE;
                                                vcanbus_send_start_cmd(moudletmp, cansend_framecnt_one[prenode->ctrlIndex - 2], prenode->ctrlIndex);   
                                        }
                                        if (prenode->beltMoudleIndex == ZONE_TYPE_TWO) {
                                                cansend_framecnt_two[prenode->ctrlIndex - 2]++;
                                                moudletmp.cmd = STOP_CMD;
                                                moudletmp.moudle = ZONE_TYPE_TWO;
                                                vcanbus_send_start_cmd(moudletmp, cansend_framecnt_two[prenode->ctrlIndex - 2], prenode->ctrlIndex);
    
                                        }
                                        if (prenode->beltMoudleIndex == ZONE_TYPE_RISE) {
                                                cansend_framecnt_rise[prenode->ctrlIndex - 2]++;
                                                moudletmp.cmd = STOP_CMD;
                                                moudletmp.moudle = ZONE_TYPE_RISE;
                                                vcanbus_send_start_cmd(moudletmp, cansend_framecnt_rise[prenode->ctrlIndex - 2], prenode->ctrlIndex);
                                        }
										//更新前一段的状态
										prestatenode->transsuccess = INVALUE;
										prestatenode->zoneState &= ~(0xF << 5);      //清除货物状态
										prestatenode->zoneState |= (1 << 5);         //置为无货状态

										prestatenode->zoneAlarm &= ~(0x3 << 9);      //清除货物报警状态
                                }  
								pkgnode->lastZoneDir = pkgnode->curZonedir;
                                pkgnode->curZonenum++;
                                pkgnode->curZoneArrive = INVALUE; 
								pkgnode->nextZonechangenum = 0;
								pkgnode->nextZonecurpkgstat = 0;
								pkgnode->nextZoneCmdValue = INVALUE;
								pkgnode->preZoneChutdown = LOGIC_TRANS_CHUTDOWNTIME;
                                continue;
                        } 


						/*if (((pkgnode->nextZonedir == RUN_AHEAD_TOBACK) && ((((nextstatenode->zoneState >> 1) & 0x1) == 1) || (((nextstatenode->zoneState >> 3) & 0x1) == 1))) ||
							((pkgnode->nextZonedir == RUN_LEFT_TOBACK) && (((nextstatenode->zoneState >> 1) & 0x1) == 1)) ||
							((pkgnode->nextZonedir == RUN_RIGHT_TOBACK) && (((nextstatenode->zoneState >> 3) & 0x1) == 1))) {
							pkgnode->curZonenum++;
							pkgnode->curZoneArrive = INVALUE;
							pkgnode->nextZonechangenum = 0;
							pkgnode->nextZonecurpkgstat = 0;
							continue;
						}

						if (((pkgnode->nextZonedir == RUN_BACK_TOAHEAD) && ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1))) ||
							((pkgnode->nextZonedir == RUN_LEFT_TOAHEAD) && (((nextstatenode->zoneState >> 4) & 0x1) == 1)) ||
							((pkgnode->nextZonedir == RUN_RIGHT_TOAHEAD) && (((nextstatenode->zoneState >> 2) & 0x1) == 1))) {
							pkgnode->curZonenum++;
							pkgnode->curZoneArrive = INVALUE;
							pkgnode->nextZonechangenum = 0;
							pkgnode->nextZonecurpkgstat = 0;
							continue;
						}


						if (((pkgnode->nextZonedir == RUN_RIGHT_TOLEFT) && ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 3) & 0x1) == 1))) ||
							((pkgnode->nextZonedir == RUN_AHEAD_TOLEFT) && (((nextstatenode->zoneState >> 3) & 0x1) == 1)) ||
							((pkgnode->nextZonedir == RUN_BACK_TOLEFT) && (((nextstatenode->zoneState >> 2) & 0x1) == 1))) {
							pkgnode->curZonenum++;
							pkgnode->curZoneArrive = INVALUE;
							pkgnode->nextZonechangenum = 0;
							pkgnode->nextZonecurpkgstat = 0;
							continue;
						}

						if (((pkgnode->nextZonedir == RUN_LEFT_TORIGHT) && ((((nextstatenode->zoneState >> 1) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1))) ||
							((pkgnode->nextZonedir == RUN_AHEAD_TORIGHT) && (((nextstatenode->zoneState >> 1) & 0x1) == 1)) ||
							((pkgnode->nextZonedir == RUN_BACK_TORIGHT) && (((nextstatenode->zoneState >> 4) & 0x1) == 1))) {
							pkgnode->curZonenum++;
							pkgnode->curZoneArrive = INVALUE;
							pkgnode->nextZonechangenum = 0;
							pkgnode->nextZonecurpkgstat = 0;
							continue;
						}*/
					}
				}

				//计算错误信息
				if (pkgnode->curZoneCnt > LOGIC_TRANS_MAXTIME) {
					curstatenode->zoneAlarm |= (0x2 << 9);      //出货超时
				}
				if (pkgnode->nextZoneCnt > LOGIC_TRANS_MAXTIME) {
					nextstatenode->zoneAlarm |= (0x2 << 9);      //接货超时
				}

				//当前区域和下一区域要发送的命令更新
				if ((pkgnode->curZonenum) >= (pkgnode->totalzoneNum - 1)) {     //最后一节皮带
					moudlecmdcur.moudle = curnode->beltMoudleIndex;
					moudlecmdcur.cmd = RUN_CMD;
					moudlecmdcur.dir = pkgnode->curZonedir;
					moudlecmdcur.type = RUN_TRIGSTOP;

					moudlecmdnext.cmd = RUN_DEFAULT;
				}
				else if((pkgnode->curZonenum) == (pkgnode->totalzoneNum - 2)){
					if (pkgnode->nextZonecurpkgstat != 0) {                           //如果最后一节皮带光电被触发了
						moudlecmdcur.moudle = curnode->beltMoudleIndex;
						moudlecmdcur.cmd = RUN_CMD;	
						moudlecmdcur.dir = pkgnode->curZonedir;
						moudlecmdcur.type = RUN_TRIGSTOP;

						moudlecmdnext.moudle = nextnode->beltMoudleIndex;
						moudlecmdnext.cmd = STOP_CMD;
						moudlecmdnext.dir = pkgnode->nextZonedir;
						moudlecmdnext.type = RUN_TRIGSTOP;
					}
					else {
						moudlecmdcur.moudle = curnode->beltMoudleIndex;
						moudlecmdcur.cmd = RUN_CMD;
						moudlecmdcur.dir = pkgnode->curZonedir;
						moudlecmdcur.type = CONTINUE_RUN;

						moudlecmdnext.moudle = nextnode->beltMoudleIndex;
						moudlecmdnext.cmd = RUN_CMD;
						moudlecmdnext.dir = pkgnode->nextZonedir;
						moudlecmdnext.type = RUN_TRIGSTOP;
					}                       
				}
				else {
					if (pkgnode->nextZonecurpkgstat != 0) {
						moudlecmdcur.moudle = curnode->beltMoudleIndex;
						moudlecmdcur.cmd = RUN_CMD;
						moudlecmdcur.dir = pkgnode->curZonedir;
						moudlecmdcur.type = RUN_TRIGSTOP;
						moudlecmdnext.moudle = nextnode->beltMoudleIndex;
						moudlecmdnext.cmd = STOP_CMD;
						moudlecmdnext.dir = pkgnode->nextZonedir;
						moudlecmdnext.type = CONTINUE_RUN;
					}
					else {
						moudlecmdcur.moudle = curnode->beltMoudleIndex;
						moudlecmdcur.cmd = RUN_CMD;
						moudlecmdcur.dir = pkgnode->curZonedir;
						moudlecmdcur.type = CONTINUE_RUN;

						moudlecmdnext.moudle = nextnode->beltMoudleIndex;
						moudlecmdnext.cmd = RUN_CMD;
						moudlecmdnext.dir = pkgnode->nextZonedir;
						moudlecmdnext.type = CONTINUE_RUN;
					}
				}

                //下一个区域光电改变过  则认为当前区域会动作在指定位置 
				//下一个区域包裹状态改变过  则认为当前区域会动作在指定位置 主要是处理顶升模块
				if (pkgnode->nextZonechangenum >= 1) {
					if (((pkgnode->curZonedir == RUN_AHEAD_TOBACK) || (pkgnode->curZonedir == RUN_LEFT_TOBACK)
						|| (pkgnode->curZonedir == RUN_RIGHT_TOBACK))) {
						moudlecmdcur.dir = RUN_AHEAD_TOBACK;
					}
					if (((pkgnode->curZonedir == RUN_BACK_TOAHEAD) || (pkgnode->curZonedir == RUN_LEFT_TOAHEAD)
						|| (pkgnode->curZonedir == RUN_RIGHT_TOAHEAD))) {
						moudlecmdcur.dir = RUN_BACK_TOAHEAD;
					}
					if (((pkgnode->curZonedir == RUN_RIGHT_TOLEFT) || (pkgnode->curZonedir == RUN_AHEAD_TOLEFT)
						|| (pkgnode->curZonedir == RUN_BACK_TOLEFT))) {
						moudlecmdcur.dir = RUN_RIGHT_TOLEFT;
					}
					if (((pkgnode->curZonedir == RUN_LEFT_TORIGHT) || (pkgnode->curZonedir == RUN_AHEAD_TORIGHT)
						|| (pkgnode->curZonedir == RUN_BACK_TORIGHT))) {
						moudlecmdcur.dir = RUN_LEFT_TORIGHT;
					}
				}

				if ((nextstatenode->zoneNextpkg != pkgnode->pkgId) || (pkgnode->curZoneArrive == INVALUE) ||
					(nextstatenode->zonePkg != 0)) {
					moudlecmdnext.cmd = RUN_DEFAULT;
					pkgnode->nextZoneCmdValue = VALUE;
				}


				//给从控制器发送指令  //区域发生切换的情况下
				if (pkgnode->curZonenum != pkgnode->lastZonenum)
				{
					pkgnode->lastZonenum = pkgnode->curZonenum;

					//给当前的模块发送指令
					if (moudlecmdcur.cmd != RUN_DEFAULT) {
						if (moudlecmdcur.moudle == ZONE_TYPE_ONE) {
							cansend_framecnt_one[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_one[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_TWO) {
							cansend_framecnt_two[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_two[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_RISE) {
							cansend_framecnt_rise[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_rise[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
					}

					//给下一个模块发送运行指令
					if (moudlecmdnext.cmd != RUN_DEFAULT) {
						if (moudlecmdnext.moudle == ZONE_TYPE_ONE) {
							cansend_framecnt_one[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_one[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_TWO) {
							cansend_framecnt_two[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_two[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_RISE) {
							cansend_framecnt_rise[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_rise[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
					}

					pkgnode->sendcmdcnt = 0;

				}

				//下一段皮带光电发生了变化
				//下一段皮带包裹状态发生了变化
				if (pkgnode->nextZonepkgstatchange == VALUE) {
					pkgnode->nextZonepkgstatchange = INVALUE;
					//给当前的模块发送运行指令
					if (moudlecmdcur.cmd != RUN_DEFAULT) {
						if (moudlecmdcur.moudle == ZONE_TYPE_ONE) {
							cansend_framecnt_one[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_one[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_TWO) {
							cansend_framecnt_two[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_two[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_RISE) {
							cansend_framecnt_rise[curnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_rise[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
					}

					//给下一个模块发送运行指令
					if (moudlecmdnext.cmd != RUN_DEFAULT) {
						if (moudlecmdnext.moudle == ZONE_TYPE_ONE) {
							cansend_framecnt_one[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_one[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_TWO) {
							cansend_framecnt_two[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_two[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_RISE) {
							cansend_framecnt_rise[nextnode->ctrlIndex - 2]++;
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_rise[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
					}
				}

				//下一个模块 刚开始允许控制
				if ((pkgnode->nextZoneCmdValue == VALUE) && (moudlecmdnext.cmd != RUN_DEFAULT)) {
					pkgnode->nextZoneCmdValue = INVALUE;

				//给下一个模块发送运行指令
					if (moudlecmdnext.moudle == ZONE_TYPE_ONE) {
						cansend_framecnt_one[nextnode->ctrlIndex - 2]++;
						vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_one[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
					}
					if (moudlecmdnext.moudle == ZONE_TYPE_TWO) {
						cansend_framecnt_two[nextnode->ctrlIndex - 2]++;
						vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_two[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
					}
					if (moudlecmdnext.moudle == ZONE_TYPE_RISE) {
						cansend_framecnt_rise[nextnode->ctrlIndex - 2]++;
						vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_rise[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
					}
				}

				//固定时间轮询发送 50ms
				if (pkgnode->sendcmdcnt >= SEND_STARTCMD_CNT) {
					pkgnode->sendcmdcnt = 0;

					
					//给当前的模块发送运行指令
					if (moudlecmdcur.cmd != RUN_DEFAULT) {
						if (moudlecmdcur.moudle == ZONE_TYPE_ONE) {
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_one[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_TWO) {
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_two[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
						if (moudlecmdcur.moudle == ZONE_TYPE_RISE) {
							vcanbus_send_start_cmd(moudlecmdcur, cansend_framecnt_rise[curnode->ctrlIndex - 2], curnode->ctrlIndex);
						}
					}

					//给下一个模块发送运行指令
					if (moudlecmdnext.cmd != RUN_DEFAULT) {
						if (moudlecmdnext.moudle == ZONE_TYPE_ONE) {
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_one[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_TWO) {
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_two[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
						if (moudlecmdnext.moudle == ZONE_TYPE_RISE) {
							vcanbus_send_start_cmd(moudlecmdnext, cansend_framecnt_rise[nextnode->ctrlIndex - 2], nextnode->ctrlIndex);
						}
					}


				}
			}
		}
	}
}





