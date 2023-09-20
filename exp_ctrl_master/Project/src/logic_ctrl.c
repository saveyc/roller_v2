#include "main.h"
#include "logic_ctrl.h"

u16 cansend_framecnt_one[BELT_ZONE_NUM] = {0};
u16 cansend_framecnt_two[BELT_ZONE_NUM] = {0};
u16 cansend_framecnt_rise[BELT_ZONE_NUM] = {0};



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
				//更新前一个区域的信息
				if (pkgnode->curZonenum != 0) {
					prenode = data_find_zone_moudlestate(pkgnode->zoneindex[pkgnode->curZonenum - 1]);
					prestatenode = data_find_ctrl_status(prenode->ctrlIndex, prenode->beltMoudleIndex);
					prestatenode->zonePkg = 0;
					prestatenode->transsuccess = INVALUE;
					prestatenode->zoneState &= 0xFE3F;      //无货
				}
				else {
					prenode = NULL;
				}

			    //更新当前区域信息
				pkgnode->curZoneIndex = pkgnode->zoneindex[pkgnode->curZonenum];
				curnode = data_find_zone_moudlestate(pkgnode->curZoneIndex);
				curstatenode = data_find_ctrl_status(curnode->ctrlIndex,curnode->beltMoudleIndex);



				//更新下一区域信息
				if ((pkgnode->curZonenum) >= (pkgnode->totalzoneNum - 1)) {
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


				//更新下一区域信息
				if (nextstatenode != NULL) {
					if (((pkgnode->nextZonedir == RUN_AHEAD_TOBACK) || (pkgnode->nextZonedir == RUN_LEFT_TOBACK)
						|| (pkgnode->nextZonedir == RUN_RIGHT_TOBACK))) {
						nextpkgstat = ((((nextstatenode->zoneState >> 1) & 0x1) == 1)|| (((nextstatenode->zoneState >> 3) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}

					if (((pkgnode->nextZonedir == RUN_BACK_TOAHEAD) || (pkgnode->nextZonedir == RUN_LEFT_TOAHEAD)
						|| (pkgnode->nextZonedir == RUN_RIGHT_TOAHEAD)) ) {
						nextpkgstat = ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
						
					}
					if (((pkgnode->nextZonedir == RUN_RIGHT_TOLEFT) || (pkgnode->nextZonedir == RUN_AHEAD_TOLEFT)
						|| (pkgnode->nextZonedir == RUN_BACK_TOLEFT))) {
						nextpkgstat = ((((nextstatenode->zoneState >> 2) & 0x1) == 1) || (((nextstatenode->zoneState >> 3) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
						
					}
					if (((pkgnode->nextZonedir == RUN_LEFT_TORIGHT) || (pkgnode->nextZonedir == RUN_AHEAD_TORIGHT)
						|| (pkgnode->nextZonedir == RUN_BACK_TORIGHT))) {
						nextpkgstat = ((((nextstatenode->zoneState >> 1) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1));
						if (pkgnode->nextZonecurpkgstat != nextpkgstat) {
							pkgnode->nextZonecurpkgstat = nextpkgstat;
							pkgnode->nextZonepkgstatchange = VALUE;
							pkgnode->nextZonechangenum++;
						}
					}

				}

				//当前区域状态更新
				//已经到达最后一段区域  (认为最后一段非顶升模块)
				if ((pkgnode->curZonenum) == (pkgnode->totalzoneNum - 1)) {
					curstatenode->zoneState &= 0xFE5F;
					curstatenode->zonePkg = pkgnode->pkgId;
					if (((((curstatenode->zoneState >> 2) & 0x1) == 1) && (pkgnode->curZonedir == RUN_BACK_TOAHEAD))
						|| ((((curstatenode->zoneState >> 1) & 0x1) == 1) && (pkgnode->curZonedir == RUN_AHEAD_TOBACK))) {
						     
						pkgnode->allowState = TRANS_SUCCESS;
						//上传运输结果
						resultnode.pkgid = pkgnode->pkgId;
						resultnode.result = TRANS_SUCCESS;
						DataaddPkgQueue(resultnode);
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
							if (prenode != NULL) {
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
							}
							curstatenode->zonePkg = 0;
							curstatenode->zoneState &= 0;
//							curstatenode->transsuccess = VALUE;

						}
                        //需要考虑成功分拣以后 该上传和执行什么操作
						uxListRemove(pkgnode->index);
						continue;
//						return;
					}
				}
				else {
					// curstatenode->zoneState &= 0xFF17;
					curstatenode->zoneState &= 0xFE5F;
					curstatenode->zonePkg = pkgnode->pkgId;
					if (((pkgnode->curZonedir == RUN_AHEAD_TOBACK) || (pkgnode->curZonedir == RUN_LEFT_TOBACK)
						|| (pkgnode->curZonedir == RUN_RIGHT_TOBACK)) && ((((curstatenode->zoneState >> 1) & 0x1) == 1) 
							|| (((curstatenode->zoneState >> 3) & 0x1) == 1))) {
						curstatenode->zoneState &= 0xFE9F;
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if (prenode != NULL) {
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
							}
							pkgnode->curZoneArrive = VALUE;
						}
					}
					if (((pkgnode->curZonedir == RUN_BACK_TOAHEAD) || (pkgnode->curZonedir == RUN_LEFT_TOAHEAD)
						|| (pkgnode->curZonedir == RUN_RIGHT_TOAHEAD)) && ((((curstatenode->zoneState >> 2) & 0x1) == 1) 
							|| (((curstatenode->zoneState >> 4) & 0x1) == 1))) {
						curstatenode->zoneState &= 0xFE9F;
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if (prenode != NULL) {
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
							}
							pkgnode->curZoneArrive = VALUE;
						}
					}
					if (((pkgnode->curZonedir == RUN_RIGHT_TOLEFT) || (pkgnode->curZonedir == RUN_AHEAD_TOLEFT)
						|| (pkgnode->curZonedir == RUN_BACK_TOLEFT)) && ((((curstatenode->zoneState >> 2) & 0x1) == 1)
							|| (((curstatenode->zoneState >> 3) & 0x1) == 1))) {
						curstatenode->zoneState &= 0xFE9F;
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if (prenode != NULL) {
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
							}
							pkgnode->curZoneArrive = VALUE;
						}
					}
					if (((pkgnode->curZonedir == RUN_LEFT_TORIGHT) || (pkgnode->curZonedir == RUN_AHEAD_TORIGHT)
						|| (pkgnode->curZonedir == RUN_BACK_TORIGHT)) && ((((curstatenode->zoneState >> 1) & 0x1) == 1)
							|| (((curstatenode->zoneState >> 4) & 0x1) == 1))) {
						curstatenode->zoneState &= 0xFE9F;
						if (pkgnode->curZoneArrive == INVALUE) {
							//给前一段模块发停止指令
							if (prenode != NULL) {
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
							}
							pkgnode->curZoneArrive = VALUE;
						}
					}
				}

				//更新下一个区域的状态
				if (pkgnode->nextZoneIndex != FINAL_CAR) {
					nextstatenode->transsuccess = INVALUE;
					nextstatenode->zonePkg = 0;
//					nextstatenode->zoneState  &= 0xFF47;
					nextstatenode->zoneState &= 0xFF1F;
                                         
                                        
                    if (pkgnode->nextZonecurpkgstat == 0) {
						//判断并切换区域
                        if ((((nextstatenode->zoneState >> 1) & 0x1) == 1) || (((nextstatenode->zoneState >> 2) & 0x1) == 1)
                                || (((nextstatenode->zoneState >> 3) & 0x1) == 1) || (((nextstatenode->zoneState >> 4) & 0x1) == 1)) {
    
                                if (prenode != NULL) {
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
                                }
    
                                pkgnode->curZonenum++;
                                pkgnode->curZoneArrive = INVALUE; 
								pkgnode->nextZonechangenum = 0;
                                continue;
//						return;
                        }                                          
					}

				}

				//当前区域要发送的命令更新
				if ((pkgnode->curZonenum) >= (pkgnode->totalzoneNum - 1)) {
					moudlecmdcur.moudle = curnode->beltMoudleIndex;
					moudlecmdcur.cmd = RUN_CMD;
					moudlecmdcur.dir = pkgnode->curZonedir;
					moudlecmdcur.type = RUN_TRIGSTOP;					
					moudlecmdnext.cmd = RUN_DEFAULT;

				}
				else if((pkgnode->curZonenum) == (pkgnode->totalzoneNum - 2)){
					if (pkgnode->nextZonecurpkgstat != 0) {
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

				if (pkgnode->curZonenum != pkgnode->lastZonenum)
				{
					pkgnode->lastZonenum = pkgnode->curZonenum;

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

					pkgnode->sendcmdcnt = 0;

				}

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





