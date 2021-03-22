/** @file
 * @brief パンドラデータセット (PDS) 関連の宣言
 */

extern int nuspds_cfglex(const unsigned char *text, size_t textsize);
extern int nuspds_dsfound(int nrd, char nustype[16],
		const char *server, unsigned port, const char *path);
