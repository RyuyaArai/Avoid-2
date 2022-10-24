#include "DxLib.h"
#include "math.h"

const char TITLE[] = "K020G1223新井龍也：Avoid!2";

const int WIN_WIDTH = 1000; //ウィンドウ横幅
const int WIN_HEIGHT = 720; //ウィンドウ縦幅


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                   _In_ int nCmdShow) {
	ChangeWindowMode(TRUE); //ウィンドウモードに設定
	//ウィンドウサイズを手動では変更できず、かつウィンドウサイズに合わせて拡大できないようにする
	SetWindowSizeChangeEnableFlag(FALSE, FALSE);
	SetMainWindowText(TITLE); // タイトルを変更
	SetGraphMode(WIN_WIDTH, WIN_HEIGHT, 32); //画面サイズの最大サイズ、カラービット数を設定（モニターの解像度に合わせる）
	SetWindowSizeExtendRate(1.0); //画面サイズを設定（解像度との比率で設定）
	SetBackgroundColor(0x00, 0x00, 0x00); // 画面の背景色を設定する

	//Dxライブラリの初期化
	if (DxLib_Init() == -1) { return -1; }

	//（ダブルバッファ）描画先グラフィック領域は裏面を指定
	SetDrawScreen(DX_SCREEN_BACK);

	//画像などのリソースデータの変数宣言と読み込み

	//画像読み込み
	int gazou[9];
	int Player[12];
	int E1DE[13]; //Enemy1DeathEffect
	int E2DE[13]; //Enemy2DeathEffect

	gazou[0] = LoadGraph("PlayerLeft.png");
	gazou[1] = LoadGraph("teki01.png");
	gazou[2] = LoadGraph("teki02.png");
	gazou[3] = LoadGraph("PlayerRight.png");
	gazou[4] = LoadGraph("Avoid-Start1.png");
	gazou[5] = LoadGraph("Avoid-Start2.png");
	gazou[6] = LoadGraph("Avoid-End.png");
	gazou[7] = LoadGraph("teki01_damage.png");
	gazou[8] = LoadGraph("teki02_damage.png");

	LoadDivGraph("player.png", 12, 12, 1, 64, 64, Player);
	LoadDivGraph("Enemy1DeathEffect.png", 13, 13, 1, 64, 64, E1DE);
	LoadDivGraph("Enemy2DeathEffect.png", 13, 13, 1, 128, 128, E2DE);

	int PLShotGraph = LoadGraph("jikidan2-2.png");

	int haikei = LoadGraph("haikei1.png");
	int haikei2 = LoadGraph("haikei2.png");

	//ゲームループで使う変数の宣言
	char keys[256] = {0}; //最新のキーボード情報用
	char oldkeys[256] = {0}; //1ループ（フレーム）前のキーボード情報

	//背景関係
	int haikeiposY = 0;

	//自機関係
	int PLposx = 300;
	int PLposy = 600;
	int PLr = 16;
	int PLw, PLh;
	GetGraphSize(Player[0], &PLw, &PLh);
	int PLDeathFlag = 0;
	int AT = 0;


	//弾変数
	int PLShotFlag = 0;
	int Shotx, Shoty;
	int Shotr = 8;
	int SHw, SHh;
	GetGraphSize(PLShotGraph, &SHw, &SHh);

	//敵機1関係
	int EnemyPosX = 300;
	int EnemyPosY = 100;
	int EnemyDeathFlag = 0;
	int EnemyR = 32;
	int EnemyTimer = 120;
	int EnemyW, EnemyH;
	GetGraphSize(gazou[1], &EnemyW, &EnemyH);
	int EnemyHP = 20;
	int EnemySPDx = 5;
	int EnemySPDy = 10;
	int E1DEAT = 0; //Enemy1DeathEffectAnimationTimer
	int E1Hit = 0, E1HitAT = 0;

	//敵機2関係
	int Enemy2PosX = 400;
	int Enemy2PosY = 35;
	int Enemy2DeathFlag = 2;
	int Enemy2R = 64;
	int Enemy2Timer = 490;
	int Enemy2W, Enemy2H;
	GetGraphSize(gazou[2], &Enemy2W, &Enemy2H);
	int Enemy2HP = 40;
	int Enemy2SPDx = 10;
	int Enemy2SPDy = 5;
	int E2DEAT = 0; //Enemy2DeathEffectAnimationTimer
	int E2Hit = 0, E2HitAT = 0;

	//座標表記色
	int PrintColor;
	PrintColor = GetColor(0xFF, 0xFF, 0xFF);

	//スコア用
	int Score = 0;
	int HighScore = 0;
	int ST = 0; //ScoreTimer

	//ゲームタイマー
	int GTFPS = 0; //GTFPS 50 = 1Second
	int GTS = 0; //Second
	int GTM = 0; //Minute
	int GTH = 0; //Hour

	int State = 0;
	int Start = 0;
	int End = 0;

	//音楽関係
	int opBgmHandle;
	opBgmHandle = LoadSoundMem("bo-tto_hidamari.mp3");
	ChangeVolumeSoundMem(255 * 20 / 100, opBgmHandle);
	int GameBgmHandle;
	GameBgmHandle = LoadSoundMem("game.mp3");
	ChangeVolumeSoundMem(255 * 10 / 100, GameBgmHandle);
	int EdBgmHandle;
	EdBgmHandle = LoadSoundMem("ED.mp3");
	ChangeVolumeSoundMem(255 * 10 / 100, EdBgmHandle);
	int SE;
	SE = LoadSoundMem("SE.ogg");
	ChangeVolumeSoundMem(255 * 10 / 100, SE);

	//ゲームループ
	while (true) {
		//最新のキーボード情報だったものは１フレーム前のキーボード情報として保存
		for (int i = 0; i < 256; ++i) {
			oldkeys[i] = keys[i];
		}
		//最新のキーボード情報を取得
		GetHitKeyStateAll(keys);

		//画面クリア
		ClearDrawScreen();

		//---------  ここからプログラムを記述  ----------//


		//Start
		if (State == 0) {
			//スコアとタイマーのリセット
			Score = 0;
			ST = 0;
			GTFPS = 0;
			GTS = 0;
			GTM = 0;
			GTH = 0;
			//更新処理
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				Start = 1;
			}
			else if (Start == 1) {
				WaitTimer(800);
				State = 1;
				Start = 0;
			}

			//描画処理

			if (keys[KEY_INPUT_SPACE] == 0 && oldkeys[KEY_INPUT_SPACE] == 0) {
				DrawGraph(0, 0, gazou[4], TRUE);
			}
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				DrawGraph(0, 0, gazou[5], TRUE);
			}
		}


		//Game
		if (State == 1) {
			//更新処理

			//自機移動処理
			if (keys[KEY_INPUT_RIGHT] == 1
				|| keys[KEY_INPUT_UP] == 1
				|| keys[KEY_INPUT_LEFT] == 1
				|| keys[KEY_INPUT_DOWN] == 1
				|| keys[KEY_INPUT_W] == 1
				|| keys[KEY_INPUT_A] == 1
				|| keys[KEY_INPUT_S] == 1
				|| keys[KEY_INPUT_D] == 1) {
				if (keys[KEY_INPUT_RIGHT] == 1
					|| keys[KEY_INPUT_D] == 1) {
					PLposx += 4;
				}
				if (keys[KEY_INPUT_UP] == 1
					|| keys[KEY_INPUT_W] == 1) {
					PLposy -= 4;
				}
				if (keys[KEY_INPUT_LEFT] == 1
					|| keys[KEY_INPUT_A] == 1) {
					PLposx -= 4;
				}
				if (keys[KEY_INPUT_DOWN] == 1
					|| keys[KEY_INPUT_S] == 1) {
					PLposy += 4;
				}
				if (PLposx <= 10) {
					PLposx += 4;
				}
				if (PLposx >= 610) {
					PLposx -= 4;
				}
				if (PLposy >= 640) {
					PLposy -= 4;
				}
				if (PLposy <= 18) {
					PLposy += 4;
				}
				if ((keys[KEY_INPUT_LSHIFT] == 1)
					&& (keys[KEY_INPUT_RIGHT] == 1
						|| keys[KEY_INPUT_UP] == 1
						|| keys[KEY_INPUT_LEFT] == 1
						|| keys[KEY_INPUT_DOWN] == 1
						|| keys[KEY_INPUT_W] == 1
						|| keys[KEY_INPUT_A] == 1
						|| keys[KEY_INPUT_S] == 1
						|| keys[KEY_INPUT_D] == 1)) {
					if (keys[KEY_INPUT_LSHIFT] == 1
						&& (keys[KEY_INPUT_RIGHT] == 1
							|| keys[KEY_INPUT_D] == 1)) {
						PLposx -= 2;
					}
					if (keys[KEY_INPUT_LSHIFT] == 1
						&& (keys[KEY_INPUT_UP] == 1
							|| keys[KEY_INPUT_W] == 1)) {
						PLposy += 2;
					}
					if (keys[KEY_INPUT_LSHIFT] == 1
						&& (keys[KEY_INPUT_LEFT] == 1)
						|| keys[KEY_INPUT_A] == 1) {
						PLposx += 2;
					}
					if (keys[KEY_INPUT_LSHIFT] == 1
						&& (keys[KEY_INPUT_DOWN] == 1)
						|| keys[KEY_INPUT_S] == 1) {
						PLposy -= 2;
					}
				}
			}

			//自機更新
			if (PLDeathFlag == 0) {
				AT++;
			}
			if (AT == 12) {
				AT = 0;
			}

			/*ショット関係処理*/
			//スペースを押して、弾が見えていなければ発射する
			if (keys[KEY_INPUT_SPACE] == 1
				&& PLShotFlag == 0) {
				Shotx = (PLw - SHw) / 2 + PLposx;
				Shoty = (PLh - SHh) / 2 + PLposy;
				PLShotFlag = 1;
			}

			//自機の弾の軌道
			if (PLShotFlag == 1) {
				//弾を16ドット上に移動
				Shoty -= 20;

				if (Shoty < 30) {
					PLShotFlag = 0;
				}
			}

			//自機と敵の衝突判定
			if (EnemyDeathFlag == 0 && PLDeathFlag == 0) {
				int PLDeJuX, PLDeJuY; //DeathJudge
				PLDeJuX = ((EnemyPosX + (EnemyW / 2)) - (PLposx + (PLw / 2)));
				PLDeJuY = ((EnemyPosY + (EnemyH / 2)) - (PLposy + (PLh / 2)));
				if (sqrtf(PLDeJuX * PLDeJuX + PLDeJuY * PLDeJuY) <= (PLr + EnemyR)) {
					PLDeathFlag = 1;
				}
			}
			if (Enemy2DeathFlag == 0 && PLDeathFlag == 0) {
				int PLDeJu2X, PLDeJu2Y;
				PLDeJu2X = ((Enemy2PosX + (Enemy2W / 2)) - (PLposx + (PLw / 2)));
				PLDeJu2Y = ((Enemy2PosY + (Enemy2H / 2)) - (PLposy + (PLh / 2)));
				if (sqrtf(PLDeJu2X * PLDeJu2X + PLDeJu2Y * PLDeJu2Y) <= (PLr + Enemy2R)) {
					PLDeathFlag = 1;
				}
			}

			//PLが死んだ場合のシーン移動
			if (PLDeathFlag == 1) {
				State = 2;
			}

			//敵1の処理
			if (EnemyDeathFlag == 0) {
				if (GTM > 0 || (GTH > 1 && GTM == 0)) {
					EnemyPosX = EnemyPosX + EnemySPDx;
					EnemyPosY = EnemyPosY + EnemySPDy;
				}
				else {
					EnemyPosX = EnemyPosX + EnemySPDx;
				}
			}
			if (EnemyPosX > 588) {
				EnemySPDx = -EnemySPDx;
			}
			if (EnemyPosX < 34) {
				EnemySPDx = -EnemySPDx;
			}
			if (EnemyPosY < 30) {
				EnemySPDy = -EnemySPDy;
			}
			if (EnemyPosY > 634) {
				EnemySPDy = -EnemySPDy;
			}

			//敵1と弾との衝突判定
			if (PLShotFlag == 1 && EnemyDeathFlag == 0) {
				int ShJudgeX, ShJudgeY;
				ShJudgeX = (EnemyPosX + (EnemyW / 2) - Shotx);
				ShJudgeY = (EnemyPosY + (EnemyH / 2) - Shoty);
				if (sqrtf(ShJudgeX * ShJudgeX + ShJudgeY * ShJudgeY) <= (Shotr + EnemyR)) {
					PLShotFlag = 0;
					if (EnemyHP > 1) {
						Score += 20;
						EnemyHP--;
					}
					else if (EnemyHP == 1) {
						Score += 120;
						EnemyHP--;
						EnemyDeathFlag = 1;
						EnemyTimer = 0;

					}
					E1Hit = 1;
				}
			}
			if (E1Hit == 1) {
				if (10 > E1HitAT) {
					E1HitAT++;
				}
				else {
					E1Hit = 0;
					if (E1Hit == 0) {
						E1HitAT = 0;
					}
				}
			}
			//DeathEffect更新
			if (EnemyHP == 0 && E1DEAT < 13) {
				E1DEAT++;
			}

			//敵1リスポーン
			if (EnemyDeathFlag == 1) {
				EnemyTimer++;
			}
			if (EnemyTimer > 118 && EnemyTimer < 120) {
				EnemyHP = 20;
				EnemyPosX = 300;
				EnemyPosY = 100;
			}
			if (EnemyTimer == 120) {
				EnemyDeathFlag = 0;
				E1DEAT = 0;
			}

			//敵2処理
			if (Enemy2DeathFlag == 0) {
				Enemy2PosX = Enemy2PosX + Enemy2SPDx;
				Enemy2PosY = Enemy2PosY + Enemy2SPDy;
			}
			if (Enemy2PosX < 30) {
				Enemy2SPDx = -Enemy2SPDx;
			}
			if (Enemy2PosX > 532) {
				Enemy2SPDx = -Enemy2SPDx;
			}
			if (Enemy2PosY < 20) {
				Enemy2SPDy = -Enemy2SPDy;
			}
			if (Enemy2PosY > 572) {
				Enemy2SPDy = -Enemy2SPDy;
			}

			//敵2と弾との衝突判定
			if (PLShotFlag == 1 && Enemy2DeathFlag == 0) {
				int ShJudge2X, ShJudge2Y;
				ShJudge2X = (Enemy2PosX + (Enemy2W / 2) - Shotx);
				ShJudge2Y = (Enemy2PosY + (Enemy2H / 2) - Shoty);
				if (sqrtf(ShJudge2X * ShJudge2X + ShJudge2Y * ShJudge2Y) <= (Shotr + Enemy2R)) {
					PLShotFlag = 0;
					if (Enemy2HP > 1) {
						Score += 10;
						Enemy2HP--;
					}
					else if (Enemy2HP == 1) {
						Score += 210;
						Enemy2HP--;
						Enemy2DeathFlag = 1;
					}
					E2Hit = 1;
				}
			}
			if (E2Hit == 1) {
				if (10 > E2HitAT) {
					E2HitAT++;
				}
				else {
					E2Hit = 0;
					if (E2Hit == 0) {
						E2HitAT = 0;
					}
				}
			}
			//DeathEffect更新
			if (Enemy2HP == 0 && E2DEAT < 13) {
				E2DEAT++;
			}

			//敵2スポーン&リスポーン
			if (GTS > 29 && Enemy2DeathFlag == 2) {
				Enemy2DeathFlag = 1;
			}
			if (Enemy2DeathFlag == 1) {
				Enemy2Timer++;
			}
			if (Enemy2Timer > 498 && Enemy2Timer < 500) {
				Enemy2HP = 40;
				Enemy2PosX = 35;
			}
			if (Enemy2Timer == 500) {
				Enemy2DeathFlag = 0;
				E2DEAT = 0;
				Enemy2Timer = 0;
			}

			//2体とも死んでいるとき+スコア
			if (EnemyDeathFlag == 1 && Enemy2DeathFlag == 1) {
				Score += 5;
			}

			//時間経過でスコア獲得
			ST++;
			if (ST == 50) {
				Score += 5 + GTM * 2;
				ST = 0;
			}

			//ゲームタイマー
			GTFPS++;
			if (GTFPS == 50) {
				GTS++;
				GTFPS = 0;
			}
			if (GTS == 60) {
				GTM++;
				GTS = 0;
			}
			if (GTM == 60) {
				GTH++;
				GTM = 0;
			}

			//背景更新
			if (haikeiposY >= 20 && haikeiposY < 699) {
				haikeiposY++;
			}
			else {
				haikeiposY++;
				haikeiposY = 20;
			}


			//描画処理

			//背景
			if (haikeiposY >= 20 && haikeiposY < 699) {
				DrawGraph(20, haikeiposY, haikei, TRUE);
				DrawGraph(20, haikeiposY - 680, haikei, TRUE);
			}
			else {
				DrawGraph(20, haikeiposY, haikei, TRUE);
				DrawGraph(20, haikeiposY - 680, haikei, TRUE);
			}
			DrawGraph(0, 0, haikei2, TRUE);

			DrawFormatString(700, 250, PrintColor, "EnemyHP：%d", EnemyHP);
			DrawFormatString(700, 300, PrintColor, "Enemy2HP：%d", Enemy2HP);
			DrawFormatString(700,  50, PrintColor, "High Score：%5.1d", HighScore);
			DrawFormatString(700, 100, PrintColor, "Score     ：%5.1d", Score);
			DrawFormatString(700, 150, PrintColor, "GameTimer %d:%d:%d", GTH, GTM, GTS);
			DrawFormatString(700, 500, PrintColor, "↑←↓→　or WASD");
			DrawFormatString(700, 520, PrintColor, "move");
			DrawFormatString(700, 550, PrintColor, "Space");
			DrawFormatString(700, 570, PrintColor, "Shot");


			//敵1描画
			if (EnemyHP == 0 && E1DEAT < 12) {
				DrawGraph(EnemyPosX, EnemyPosY, E1DE[E1DEAT], TRUE);
			}
			if (E1Hit == 1 && EnemyDeathFlag == 0) {
				DrawGraph(EnemyPosX, EnemyPosY, gazou[7], TRUE);
			}
			else if (EnemyDeathFlag == 0) {
				DrawGraph(EnemyPosX, EnemyPosY, gazou[1], TRUE);
			}

			//敵2描画
			if (Enemy2DeathFlag == 0) {
				DrawGraph(Enemy2PosX, Enemy2PosY, gazou[2], TRUE);
			}
			if (E2Hit == 1 && Enemy2DeathFlag == 0) {
				DrawGraph(Enemy2PosX, Enemy2PosY, gazou[8], TRUE);
			}
			else if (Enemy2HP == 0 && E2DEAT < 12) {
				DrawGraph(Enemy2PosX, Enemy2PosY, E2DE[E2DEAT], TRUE);
			}

			//画面に弾を描写
			if (PLShotFlag == 1) {
				DrawGraph(Shotx, Shoty, PLShotGraph, TRUE);
			}
			//プレイヤーのアニメーション等
			if (PLDeathFlag == 0 &&
				(keys[KEY_INPUT_LEFT]
					|| keys[KEY_INPUT_A])) {
				DrawGraph(PLposx, PLposy, gazou[0], TRUE);
			}
			else if (PLDeathFlag == 0
				&& (keys[KEY_INPUT_RIGHT]
					|| keys[KEY_INPUT_D])) {
				DrawGraph(PLposx, PLposy, gazou[3], TRUE);
			}
			else if (PLDeathFlag == 0) {
				DrawGraph(PLposx, PLposy, Player[AT], TRUE);
			}

		}


		//End
		if (State == 2) {
			if (HighScore < Score) {
				HighScore = Score;
			}
			//更新処理
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				End = 1;
			}
			else if (End == 1) {
				WaitTimer(800);
				State = 0;
				End = 0;
				//初期化
				PLposx = 300;
				PLposy = 600;
				PLDeathFlag = 0;
				EnemyHP = 20;
				EnemyPosX = 35;
				EnemyPosY = 100;
				EnemyDeathFlag = 0;
				EnemyTimer = 120;
				Enemy2HP = 40;
				Enemy2PosX = 400;
				Enemy2PosY = 35;
				Enemy2DeathFlag = 2;
				Enemy2Timer = 490;
			}

			//描画処理
			DrawGraph(0, 0, gazou[6], TRUE);
			DrawFormatString(430, 330, PrintColor, "スコア　　：%5.1d", Score);
			DrawFormatString(430, 360, PrintColor, "ハイスコア：%5.1d", HighScore);
			DrawFormatString(430, 390, PrintColor, "GameTimer %2.1d:%2.1d:%2.1d", GTH, GTM, GTS);
		}


		//音楽処理
		if (State == 0) {
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				PlaySoundMem(SE, DX_PLAYTYPE_NORMAL);
			}
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				StopSoundMem(opBgmHandle);
			}
			else if (CheckSoundMem(opBgmHandle) == 0) {
				PlaySoundMem(opBgmHandle, DX_PLAYTYPE_BACK);
			}
		}
		if (State == 1) {
			if (PLDeathFlag == 1) {
				StopSoundMem(GameBgmHandle);
			}
			else if (CheckSoundMem(GameBgmHandle) == 0) {
				PlaySoundMem(GameBgmHandle, DX_PLAYTYPE_BACK);
			}
		}
		if (State == 2) {
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				PlaySoundMem(SE, DX_PLAYTYPE_NORMAL);
			}
			if (PLDeathFlag == 1) {
				StopSoundMem(GameBgmHandle);
			}
			if (keys[KEY_INPUT_SPACE] == 1 && oldkeys[KEY_INPUT_SPACE] == 0) {
				StopSoundMem(EdBgmHandle);
			}
			else if (CheckSoundMem(EdBgmHandle) == 0) {
				PlaySoundMem(EdBgmHandle, DX_PLAYTYPE_BACK);
			}
		}
		//---------  ここまでにプログラムを記述  ---------//
		ScreenFlip(); //（ダブルバッファ）裏面
		// 20ミリ秒待機（疑似50FPS）
		WaitTimer(20);
		// Windows システムからくる情報を処理する
		if (ProcessMessage() == -1) {
			break;
		}
		// ESCキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) {
			break;
		}
	}
	//Dxライブラリ終了処理
	DxLib_End();

	return 0;
}
