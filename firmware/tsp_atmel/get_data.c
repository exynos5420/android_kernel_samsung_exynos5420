#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ERR(x...) { fprintf(stderr, x); exit(-1); }
#define PERR(x...) { perror(x); exit(-1); }

#define MXT_FW_MAGIC	0x4d3c2b1a
#define OUTPUT_CFG_FILE		"cfg_file.txt"

/* keep the format with header of firmware file */
struct atmel_fw_hdr {
    uint32_t magic_code;
    uint32_t hdr_len;
    uint32_t cfg_len;
    uint32_t fw_len;
    uint32_t cfg_crc;
    uint8_t fw_ver;
    uint8_t build_ver;
} __attribute__ ((packed));

struct atmel_cfg_hdr {
	uint8_t type;
	uint8_t instance;
	uint8_t size;
} __attribute__ ((packed));

uint32_t gen_cfg_file(const char *in_file)
{
	int ret, cnt;
	unsigned int fw_size;
	unsigned int i, j, temp;
	FILE *input, *output;
	struct stat stat;
	struct atmel_fw_hdr hdr;
	struct atmel_cfg_hdr cfg_hdr;
	unsigned char buf[512];
	unsigned char has_patch;

	input = fopen(in_file, "rb");
	if (!input)
		ERR("cannot open firmware file");

	output = fopen(OUTPUT_CFG_FILE, "w");
	if (!output)
		ERR("cannot open cfg file");

	ret = fstat(fileno(input), &stat);
	if (ret < 0)
		ERR("cannot stat fwfile");
	fw_size = stat.st_size;

	/* Read firmware image base on head info */
	cnt = fread(&hdr, sizeof(struct atmel_fw_hdr), 1, input);
	if (cnt < 0)
		ERR("cannot read firmware image");
 
	/* Check the firmware file with header info */
	if (hdr.hdr_len != sizeof(struct atmel_fw_hdr))
		ERR("Firmware file is invaild !! hdr size[%u] cfg,fw size[%u,%u] filesize[%u]\n",
				hdr.hdr_len, hdr.cfg_len, hdr.fw_len, fw_size);

	if (!hdr.cfg_len)
		ERR("Firmware file dose not include configuration data\n");
	if (!hdr.fw_len)
		ERR("Firmware file dose not include raw firmware data\n");

	/* Check the magic code */
	if (hdr.magic_code != MXT_FW_MAGIC)
		ERR("Magic code mismatched %x\n", hdr.magic_code);

	fprintf(output, "Magic Code:\t\t\t\t\t0x%x\n", hdr.magic_code);
	fprintf(output, "Header length:\t\t\t\t%u\n", hdr.hdr_len);
	fprintf(output, "Configuration length:\t\t%u\n", hdr.cfg_len);
	fprintf(output, "Frimware length:\t\t\t%u\n", hdr.fw_len);
	fprintf(output, "Configuration CRC Code:\t\t0x%x\n", hdr.cfg_crc);
	fprintf(output, "Fimware main version:\t\t0x%x\n", hdr.fw_ver);
	fprintf(output, "Firmware build version:\t\t0x%x\n", hdr.build_ver);

	/* Generate the cfg data into cfg file as below format
	 * object_num	instance	size	data...
	 */
	for (i = 0; i < hdr.cfg_len;) {
		if (i + sizeof(struct atmel_cfg_hdr) >= hdr.cfg_len)
			ERR("index(%lu) of cfg_data exceed the total size(%u)!!\n",
				i + sizeof(struct atmel_cfg_hdr), hdr.cfg_len);

		/* Read and write header of each cfg data */
		cnt = fread(&cfg_hdr, sizeof(struct atmel_cfg_hdr), 1, input);
		if (cnt < 0)
			ERR("cannot read cfg header\n");
		i += cnt * sizeof(struct atmel_cfg_hdr) + cfg_hdr.size;

		if (i > hdr.cfg_len)
			ERR("index(%d) of cfg_data exceed the total size(%d) in T%d object!!\n",
				i, hdr.cfg_len, cfg_hdr.type);

		fprintf(output, "\n\nT%02d : instance[%02d] size[%03d]\n",
			cfg_hdr.type, cfg_hdr.instance, cfg_hdr.size);

		/* Read and write cfg data */
		cnt = fread(&buf, cfg_hdr.size, 1, input);
		if (cnt < 0)
			ERR("Err read cfg header in T%02d\n", cfg_hdr.type);

		for (j = 0; j < cfg_hdr.size; j++) {
			if (j && !(j% 10))
				fprintf(output, "\t");
			if (j && !(j% 50))
				fprintf(output, "\n");

			fprintf(output, "%02x ", buf[j]);
		}
	}
	printf("Cfg file generated successfully.\n");
	fclose(output);
	fclose(input);

	/* Generate the patch data into patch file */
	if (hdr.hdr_len + hdr.cfg_len + hdr.fw_len != fw_size) {
		uint32_t ppos = hdr.hdr_len + hdr.cfg_len + hdr.fw_len;
		return ppos;
	}

	return 0;
}

/* Related patch structs can be changed....
 * so update when there are changed.
 */
#define OUTPUT_PATCH_FILE	"patch_file.txt"

/* Below structs are copied from mxts_patch.c file */
#define MXT_PATCH_MAGIC				0x52296416
#define MXT_PATCH_VERSION			1
#define MXT_PATCH_MAX_STAGE			255
#define MXT_PATCH_MAX_TLINE			255
#define MXT_PATCH_MAX_TRIG			255
#define MXT_PATCH_MAX_ITEM			255
#define MXT_PATCH_MAX_TYPE			255
#define MXT_PATCH_MAX_CON			255
#define MXT_PATCH_MAX_EVENT			255
#define MXT_PATCH_MAX_MSG_SIZE		10

#define MXT_PATCH_MAX_ITEM			255
#define MXT_PATCH_MAX_TYPE			255
#define MXT_PATCH_MAX_CON			255
#define MXT_PATCH_MAX_EVENT			255
#define MXT_PATCH_MAX_MSG_SIZE		10

#define MXT_XML_CON_NONE		"NA"			//0
#define MXT_XML_CON_EQUAL		"="			//1
#define MXT_XML_CON_BELOW		"<"			//2
#define MXT_XML_CON_ABOVE		">"			//3
#define MXT_XML_CON_PLUS		"+"			//4
#define MXT_XML_CON_MINUS		"-"			//5
#define MXT_XML_CON_MUL			"*"			//6
#define MXT_XML_CON_DIV			"/"			//7

#define MXT_XML_SRC_NONE		"NA"		//0
#define MXT_XML_SRC_CHRG		"CHRG"		//1
#define MXT_XML_SRC_FCNT		"FCNT"		//2
#define MXT_XML_SRC_AREA		"AREA"		//3
#define MXT_XML_SRC_AMP			"AMP"		//4
#define MXT_XML_SRC_SUM			"SUM"		//5
#define MXT_XML_SRC_TCH			"TCH"		//6
#define MXT_XML_SRC_ATCH		"ATCH"		//7

#define MXT_XML_ACT_NONE		"NA"			//0
#define MXT_XML_ACT_CAL			"CALIBRATION"		//1
#define MXT_XML_ACT_EXTMR		"EXTEND TIMER"		//2

enum{
	MXT_PATCH_CON_NONE = 0,
	MXT_PATCH_CON_EQUAL,		//1
	MXT_PATCH_CON_BELOW,		//2
	MXT_PATCH_CON_ABOVE,		//3
	MXT_PATCH_CON_PLUS,			//4
	MXT_PATCH_CON_MINUS,		//5
	MXT_PATCH_CON_MUL,			//6
	MXT_PATCH_CON_DIV,			//7
	//...
	MXT_PATCH_CON_END
};

enum {
	MXT_PATCH_ITEM_NONE	= 0,
	MXT_PATCH_ITEM_CHARG,		//1
	MXT_PATCH_ITEM_FCNT,		//2
	MXT_PATCH_ITEM_AREA,		//3
	MXT_PATCH_ITEM_AMP,			//4
	MXT_PATCH_ITEM_SUM,			//5
	MXT_PATCH_ITEM_TCH,			//6
	MXT_PATCH_ITEM_ATCH,		//7
	//...
	MXT_PATCH_ITEM_END
};

enum {
	MXT_PATCH_ACTION_NONE = 0,
	MXT_PATCH_ACTION_CAL,
	MXT_PATCH_ACTION_EXTEND_TIMER,
	//...
	MXT_PATCH_ACTION_END
};

struct patch_header { // 32b		
	uint32_t	magic;			
	uint32_t	size;	
	uint32_t	date;			
	uint16_t	version;			
	uint8_t		option;
	uint8_t		debug;
	uint8_t		timer_id;		
	uint8_t		stage_cnt;		
	uint8_t		trigger_cnt;
	uint8_t		event_cnt;
	uint8_t		reserved[12];
};

struct stage_def {	// 8b
	uint8_t		stage_id;				
	uint8_t		option;
	uint16_t	stage_period;
	uint8_t	cfg_cnt;
	uint8_t	test_cnt;
	uint8_t	reserved[2];
};

struct stage_cfg {	// 4b
	uint8_t	obj_type;
	uint8_t	reserved;
	uint8_t	offset;
	uint8_t	val;
};

struct test_line{	// 12b
	uint8_t	test_id;	
	uint8_t	item_cnt;
	uint8_t	cfg_cnt;			
	uint8_t	act_id;
	uint16_t act_val;
	uint16_t option;
	uint16_t check_cnt;
	uint8_t  reserved[2];
};

struct action_cfg{	// 4b
	uint8_t	obj_type;
	uint8_t	reserved;
	uint8_t	offset;
	uint8_t	val;
};
struct item_val{	// 4b
	uint8_t	val_id;
	uint8_t	val_eq;				
	uint16_t val;
};

struct test_item{	// 8b
	uint8_t	src_id;
	uint8_t	cond;
	uint8_t	reserved[2];
	struct item_val ival;
};

// Message Trigger
struct trigger{		// 12b
	uint8_t tid;
	uint8_t option;
	uint8_t object;
	uint8_t index;
	uint8_t match_cnt;
	uint8_t cfg_cnt;
	uint8_t reserved[3];
	uint8_t act_id;
	uint16_t act_val;
};

struct match{		//8b
	uint8_t offset;
	uint8_t cond;
	uint16_t mask;
	uint8_t reserved[2];
	uint16_t val;
};

struct trigger_cfg{	// 4b
	uint8_t	obj_type;
	uint8_t	reserved;
	uint8_t	offset;
	uint8_t	val;
};

// Event
struct user_event{	// 8b
	uint8_t eid;
	uint8_t option;
	uint8_t cfg_cnt;
	uint8_t reserved[5];
};

struct event_cfg{	// 4b
	uint8_t	obj_type;
	uint8_t	reserved;
	uint8_t	offset;
	uint8_t	val;
};

const char* mxt_patch_src_item_name(uint8_t src_id)
{
	const char* src_item_name[MXT_PATCH_MAX_TYPE]= {
		MXT_XML_SRC_NONE,	//MXT_PATCH_ITEM_NONE		0
		MXT_XML_SRC_CHRG,	//MXT_PATCH_ITEM_CHARGER	1
		MXT_XML_SRC_FCNT,	//MXT_PATCH_ITEM_FINGER_CNT	2
		MXT_XML_SRC_AREA,	//MXT_PATCH_ITEM_T9_AREA	3
		MXT_XML_SRC_AMP,	//MXT_PATCH_ITEM_T9_AMP		4
		MXT_XML_SRC_SUM,	//MXT_PATCH_ITEM_T57_SUM	5
		MXT_XML_SRC_TCH,	//MXT_PATCH_ITEM_T57_TCH	6
		MXT_XML_SRC_ATCH,	//MXT_PATCH_ITEM_T57_ATCH	7
	};
	if(MXT_PATCH_ITEM_NONE <= src_id && 
		src_id < MXT_PATCH_ITEM_END){
		return src_item_name[src_id];
	}
	return "ERR";
}

const char* mxt_patch_cond_name(uint8_t con_id)
{
	const char* cond_name[MXT_PATCH_MAX_CON]={
		MXT_XML_CON_NONE,	//MXT_PATCH_CON_NONE	0
		MXT_XML_CON_EQUAL,	//MXT_PATCH_CON_EQUAL	1
		MXT_XML_CON_BELOW,	//MXT_PATCH_CON_BELOW	2
		MXT_XML_CON_ABOVE,	//MXT_PATCH_CON_ABOVE	3
		MXT_XML_CON_PLUS,	//MXT_PATCH_CON_PLUS	4
		MXT_XML_CON_MINUS,	//MXT_PATCH_CON_MINUS	5
		MXT_XML_CON_MUL,	//MXT_PATCH_CON_MUL		6
		MXT_XML_CON_DIV,	//MXT_PATCH_CON_DIV		7
	};
	if(MXT_PATCH_CON_NONE <= con_id && 
		con_id < MXT_PATCH_CON_END){
		return cond_name[con_id];
	}
	return "ERR";
}

const char* mxt_patch_action_name(uint8_t act_id)
{
	const char* act_name[MXT_PATCH_MAX_CON]={
		MXT_XML_ACT_NONE,	//	MXT_PATCH_ACTION_NONE			0
		MXT_XML_ACT_CAL,	// MXT_PATCH_ACTION_CAL				1
		MXT_XML_ACT_EXTMR,	// MXT_PATCH_ACTION_EXTEND_TIMER	2
	};
	if(MXT_PATCH_ACTION_NONE <= act_id && 
		act_id < MXT_PATCH_ACTION_END){
		return act_name[act_id];
	}
	return "ERR";
}

void parsing_stage(FILE *input, FILE *output)
{
	int ret, cnt;
	unsigned int i, j;
	struct stage_def psdef;
	struct stage_cfg pscfg;

	struct test_line	ptline;
	struct test_item	ptitem;
	struct action_cfg	pacfg;

	cnt = fread(&psdef, sizeof(struct stage_def), 1, input);
	if (cnt < 0)
		ERR("cannot read stage.");

	fprintf(output,	"\nSTAGE_ID:[%d]\tOPT:%d PERIOD:%d CFG_CNT:%d TEST_CNT:%d\t",
		psdef.stage_id, psdef.option, psdef.stage_period, 
		psdef.cfg_cnt, psdef.test_cnt);

	fprintf(output, "\t\t\t\t\t\t\t\t\t\t|--- Write stage cfg values and check each test lines every %d msec.\n",
		psdef.stage_period);
	/* Parsing stage config */
	for (i = 0; i < psdef.cfg_cnt; i++) {
		cnt = fread(&pscfg, sizeof(struct stage_cfg), 1, input);
		if (cnt < 0)
			ERR("cannot read stage cfg. cnt[%d]", i);
		fprintf(output, "\t|- STAGE_CFG: OBJECT_TYPE:%d OFFSET:%d VAL:%d\n", 
			pscfg.obj_type, pscfg.offset, pscfg.val);
	}	
	fprintf(output, "\n");

	/* Parsing test line */
	for (i = 0; i < psdef.test_cnt; i++) {
		cnt = fread(&ptline, sizeof(struct test_line), 1, input);
		if (cnt < 0)
			ERR("cannot read stage test. cnt[%d]", i);
		fprintf(output, "\t|- TEST_LINE:[%d] OPT:%d CHK_CNT:%d"
			" ITEM_CNT:%d CFG_CNT:%d ACTION:%d VAL:%d\t",
			ptline.test_id, ptline.option, ptline.check_cnt, 
			ptline.item_cnt, ptline.cfg_cnt, ptline.act_id, 
			ptline.act_val);

		fprintf(output, "\t\t\t\t\t|--- If below itmes is all satisfied during %d msec, Write action cfg values and then Excute %s.\n",
			ptline.check_cnt * psdef.stage_period, mxt_patch_action_name(ptline.act_id));

		/* Parsing test item */		
		for (j = 0; j < ptline.item_cnt; j++) {
			cnt = fread(&ptitem, sizeof(struct test_item), 1, input);
			if (cnt < 0)
				ERR("cannot read stage test item. cnt[%d]", j);
			fprintf(output, "\t\t|-- ITEM:[%d] SRC_ID:%s COND:%s "
				"ITEM_VAL[VAL_ID:%s VAL_EQ:%s VAL:%d]  \t\t", j,
				mxt_patch_src_item_name(ptitem.src_id),
				mxt_patch_cond_name(ptitem.cond),
				mxt_patch_src_item_name(ptitem.ival.val_id),
				mxt_patch_cond_name(ptitem.ival.val_eq),
				ptitem.ival.val);

			/* Interpret the test item */
			switch (ptitem.cond) {
			case MXT_PATCH_CON_EQUAL:	
				fprintf(output, "\t\t\t|--- IF %s vaule is == %s%s%d\n", 
					mxt_patch_src_item_name(ptitem.src_id),
					ptitem.ival.val_id ? mxt_patch_src_item_name(ptitem.ival.val_id) : "",
					ptitem.ival.val_eq ? mxt_patch_cond_name(ptitem.ival.val_eq) : "",
					ptitem.ival.val);
			break;
			case MXT_PATCH_CON_BELOW:
				fprintf(output, "\t\t\t|--- IF %s value is < %s%s%d\n", 
					mxt_patch_src_item_name(ptitem.src_id),
					ptitem.ival.val_id ? mxt_patch_src_item_name(ptitem.ival.val_id) : "",
					ptitem.ival.val_eq ? mxt_patch_cond_name(ptitem.ival.val_eq) : "",
					ptitem.ival.val);
			break;
			case MXT_PATCH_CON_ABOVE:
				fprintf(output, "\t\t\t|--- IF %s value is > %s%s%d\n", 
					mxt_patch_src_item_name(ptitem.src_id),
					ptitem.ival.val_id ? mxt_patch_src_item_name(ptitem.ival.val_id) : "",
					ptitem.ival.val_eq ? mxt_patch_cond_name(ptitem.ival.val_eq) : "",
					ptitem.ival.val);
			break;
			default:
				fprintf(output, "\t\t\t@@ INVALID TEST COND=%d !!\n", 
					ptitem.cond);
			break;
			}
		}
		
		/* Parsing test line action config*/		
		for (j = 0; j < ptline.cfg_cnt; j++) {
			cnt = fread(&pacfg, sizeof(struct action_cfg), 1, input);
			if (cnt < 0)
				ERR("cannot read stage test line action config. cnt[%d]", j);
			fprintf(output, "\t\t|-- ACTION_CFG: OBJECT_TYPE:%d"
				" OFFSET:%d VAL:%d\t\t", 
				pacfg.obj_type, pacfg.offset, pacfg.val);
			fprintf(output, "\t\t\t\t\t\t\t\t\t|--- Write T%d's %dth byte with 0x%02X\n", 
				pacfg.obj_type, pacfg.offset, pacfg.val);

		}
		fprintf(output, "\n");
	}	
}

void parsing_trigger(FILE *input, FILE *output)
{
	int ret, cnt;
	unsigned int i;

	struct trigger ptrgg;
	struct match pmatch;
	struct trigger_cfg ptcfg;

	cnt = fread(&ptrgg, sizeof(struct trigger), 1, input);
	if (cnt < 0)
		ERR("cannot read trigger");

	fprintf(output,"TRIGGER ID:[%d] OPT:%d OBJ:%d IDX:%d "
			"MATCH:%d CFG:%d ACTION:[%s] VAL:%d  OPT[0/1 0:non chager, 1: chager]\n\n",
			ptrgg.tid, ptrgg.option, ptrgg.object, 
			ptrgg.index, ptrgg.match_cnt, ptrgg.cfg_cnt, 
			mxt_patch_action_name(ptrgg.act_id), ptrgg.act_val);

	/* Parsing match */
	for (i = 0; i < ptrgg.match_cnt; i++) {
		cnt = fread(&pmatch, sizeof(struct match), 1, input);
		if (cnt < 0)
			ERR("cannot read match. cnt[%d]", i);
		fprintf(output, "\t|- MATCH:[%d] OFFSET:%d MASK:%d"
				" COND:%s VAL:%d\t\t", i,
				pmatch.offset, pmatch.mask, 
				mxt_patch_cond_name(pmatch.cond), pmatch.val);
		fprintf(output, "\t\t\t\t\t\t\t\t\t\t\t|--- If T%d's message[%d] & 0x%02X is value 0x%02X, Write trigger cfg values.\n", 
				ptrgg.object, pmatch.offset, pmatch.mask, pmatch.val);

	}	
	fprintf(output, "\n");

	// Parsing trigger config */
	for (i = 0; i < ptrgg.cfg_cnt; i++) {
		cnt = fread(&ptcfg, sizeof(struct trigger_cfg), 1, input);
		if (cnt < 0)
			ERR("cannot read stage trigger config. cnt[%d]", i);

		fprintf(output, "\t|- TRIGGER_CFG: OBJECT_TYPE:%d \t"
				" OFFSET:%d \tVAL:%d\n", 
				ptcfg.obj_type, ptcfg.offset, ptcfg.val);
	}
	fprintf(output, "\n");
}

void parsing_event(FILE *input, FILE *output)
{
	int ret, cnt;
	unsigned int i, j;

	struct user_event pevent;
	struct event_cfg pecfg;

	cnt = fread(&pevent, sizeof(struct user_event), 1, input);
	if (cnt < 0)
		ERR("cannot read event");

	fprintf(output, "EVENT ID:[%d] OPT:%d CFG:%d\n",
			pevent.eid, pevent.option, pevent.cfg_cnt);

	/* Parsing event config */
	for (i = 0; i < pevent.cfg_cnt; i++) {
		cnt = fread(&pecfg, sizeof(struct event_cfg), 1, input);
		if (cnt < 0)
			ERR("cannot read match. cnt[%d]", i);
		fprintf(output,"\t|- EVENT_CFG: "
				"OBJECT_TYPE:%d OFFSET:%d VAL:%d\n", 
				pecfg.obj_type, pecfg.offset, pecfg.val);
	}	
	fprintf(output, "\n");
}

void gen_patch_file(const char *in_file, uint32_t ppos)
{
	int ret, cnt;
	unsigned int i;
	FILE *input, *output;
	struct patch_header hdr;
	unsigned char buf[512];

	input = fopen(in_file, "rb");
	if (!input)
		ERR("cannot open firmware file");

	output = fopen(OUTPUT_PATCH_FILE, "w");
	if (!output)
		ERR("cannot open patch file");

	/* Move to patch header position */
	cnt = fseek(input, ppos, 0);
	if (cnt < 0)
		ERR("cannot seek position [%u]", ppos);
	
	/* Read the patch header info */
	cnt = fread(&hdr, sizeof(struct patch_header), 1, input);
	if (cnt < 0)
		ERR("cannot read firmware image");

	/* Check the magic code */
	if (hdr.magic != MXT_PATCH_MAGIC)
		ERR("Magic code mismatched %x\n", hdr.magic);

	/* Read and write patch header */
	fprintf(output, "Magic Code:\t\t\t0x%x\n", hdr.magic);
	fprintf(output, "Size:\t\t\t\t%u\n", hdr.size);
	fprintf(output, "Date:\t\t\t\t%u\n", hdr.date);
	fprintf(output, "Version:\t\t\t%u\n", hdr.version);
	fprintf(output, "Option:\t\t\t\t%d\n", hdr.option);
	fprintf(output, "Debug:\t\t\t\t%d\n", hdr.debug);
	fprintf(output, "Timer_id:\t\t\t%d\n", hdr.timer_id);
	fprintf(output, "Stage_cnt:\t\t\t%d\n", hdr.stage_cnt);
	fprintf(output, "Trigger_cnt:\t\t%d\n", hdr.trigger_cnt);
	fprintf(output, "Eevent_ent:\t\t\t%d\n\n\n", hdr.event_cnt);

	/* Parsing stage def */
	fprintf(output, "###########################################################################\n");
	fprintf(output, "########                       Parsing stage                       ########\n");
	fprintf(output, "###########################################################################\n\n");
	for (i = 0; i < hdr.stage_cnt; i++)
		parsing_stage(input, output);

	/* Parsing trigger */
	fprintf(output, "###########################################################################\n");
	fprintf(output, "########                       Parsing trigger                     ########\n");
	fprintf(output, "###########################################################################\n\n");
	for (i = 0; i < hdr.trigger_cnt; i++)
		parsing_trigger(input, output);

	fprintf(output, "###########################################################################\n");
	fprintf(output, "########               Parsing event(current not used)             ########\n");
	fprintf(output, "###########################################################################\n\n");
	/* Parsing event */
	for (i = 0; i < hdr.event_cnt; i++)
		parsing_event(input, output);

	printf("Patch file generated successfully.\n");
	fclose(output);
	fclose(input);

	return;
}

int main(int argc, char **argv)
{
	int opt;
	char *in_file = NULL;
	uint32_t ppos = 0;

	while ((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch (opt) {
		case 'i':
			in_file = strdup(optarg);
			break;

		default: /* '?' */
			fprintf(stderr,
				"Usage: %s -f <fw file>\n",
				argv[0]);
		}
	}

	if (!in_file)
		ERR("required arguments not provided\n");
	
	ppos = gen_cfg_file(in_file);

	if (ppos)
		gen_patch_file(in_file, ppos);

	return 0;
}

