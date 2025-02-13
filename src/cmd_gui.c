﻿/* -*- coding: utf-8-with-signature; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  - 2022/07/29 作成
 */

#include "suika.h"

static bool init(void);
static bool cleanup(void);

/*
 * GUIコマンド
 */
bool gui_command(int *x, int *y, int *w, int *h)
{
	/* 最初のフレームの場合 */
	if (!is_in_command_repetition()) {
		/* 初期化を行う */
		if (!init())
			return false;

		/* 最初のフレームの描画を行う */
		if (!run_gui_mode(x, y, w, h))
			return false;

		/* 次のフレーム以降はmain.cでrun_gui_mode()が呼ばれる */
		return true;
	}

	/*
	 * GUIモードが終了した際にここに到達する
	 *  - タイトルへ戻るが選択された場合、ここに到達しない
	 */
	if (!cleanup())
		return false;

	return true;
}

/* 初期化を行う */
static bool init(void)
{
	const char *file;
	bool cancel;

	/* GUIファイル名を取得する */
	file = get_string_param(GUI_PARAM_FILE);

	/* 右クリックキャンセルするか取得する */
	if (strcmp(get_string_param(GUI_PARAM_RIGHTCLICKCANCEL), "cancel")
	    == 0)
		cancel = true;
	else
		cancel = false;

	/* GUIファイルと指定された画像の読み込みを行う */
	if (!prepare_gui_mode(file, cancel, false)) {
		log_script_exec_footer();
		return false;
	}

	/* 背景以外を消す */
	show_namebox(false);
	show_msgbox(false);
	set_ch_file_name(CH_BACK, NULL);
	set_ch_file_name(CH_RIGHT, NULL);
	set_ch_file_name(CH_LEFT, NULL);
	set_ch_file_name(CH_CENTER, NULL);
	change_ch_immediately(CH_BACK, NULL, 0, 0, 0);
	change_ch_immediately(CH_LEFT, NULL, 0, 0, 0);
	change_ch_immediately(CH_RIGHT, NULL, 0, 0, 0);
	change_ch_immediately(CH_CENTER, NULL, 0, 0, 0);

	/* 終了後に表示されるBGレイヤを設定する */
	if (!create_temporary_bg_for_gui()) {
		log_script_exec_footer();
		return false;
	}

	/* 繰り返し処理を開始する */
	start_command_repetition();

	/* 繰り返し処理中のままGUIモードに移行する */
	start_gui_mode();

	return true;
}

/* 終了処理を行う */
static bool cleanup(void)
{
	const char *label;
	bool ret;

	/* 繰り返し処理を終了する */
	stop_command_repetition();

	/* ラベルジャンプボタンが押下された場合 */
	label = get_gui_result_label();
	if (label != NULL) {
		ret = move_to_label(label);
		cleanup_gui();
		return ret;
	}

	/* 終了ボタンが押下された場合 */
	if (is_gui_result_exit()) {
		cleanup_gui();
		return false;
	}

	/*
	 * キャンセルボタンが押下された場合と、
	 * 右クリックでキャンセルされた場合は、次のコマンドへ移動する
	 */
	ret = move_to_next_command();
	cleanup_gui();
	return ret;
}
