
/*  parser.c
    This file is used to obtain the local symbol infos from an object file with DWARF debug infos

    The --names 
    option adds some extra printing.

    To use, try
        make
        ./simplereader simplereader
*/
#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#include <unistd.h>     /* For close() */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "dwarf.h"
#include "libdwarf.h"

#define bool int
#define true 1
#define false 0

struct srcfilesdata {
    char ** srcfiles;
    Dwarf_Signed srcfilescount;
    int srcfilesres;
};

int Error(int res, const char *szInfo, bool bWarn);
int printIndent(int);
bool IsGoodTag( Dwarf_Half tag);
static void read_cu_list(Dwarf_Debug dbg);
static void print_die_data(Dwarf_Debug dbg, Dwarf_Die print_me,int level,
   struct srcfilesdata *sf);
static void DepthFirst(Dwarf_Debug dbg, Dwarf_Die in_die,int in_level,
   struct srcfilesdata *sf);
static void resetsrcfiles(Dwarf_Debug dbg,struct srcfilesdata *sf);

static int namesoptionon = 0;

int 
main(int argc, char **argv)
{

    Dwarf_Debug dbg = 0;
    int fd = -1;
    const char *filepath = "<stdin>";
    int res = DW_DLV_ERROR;
    Dwarf_Error error;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;

    if(argc < 2) {
        fd = 0; /* stdin */
    } else {
        int i = 0;
        for(i = 1; i < (argc-1) ; ++i) {
            if(strcmp(argv[i],"--names") == 0) {
                namesoptionon=1;
            } else {
                printf("Unknown argument \"%s\" ignored\n",argv[i]);
            }
        }
        filepath = argv[i];
        fd = open(filepath,O_RDONLY);
    }
    if(argc > 2) {
    }
    if(fd < 0) {
        printf("Failure attempting to open \"%s\"\n",filepath);
    }
	// initalize 
    res = dwarf_init(fd,DW_DLC_READ,errhand,errarg, &dbg,&error);
    if(res != DW_DLV_OK) {
        printf("Giving up, cannot do DWARF processing\n");
        exit(1);
    }

    read_cu_list(dbg);
	//finalize
    res = dwarf_finish(dbg,&error);
    if(res != DW_DLV_OK) {
        printf("dwarf_finish failed!\n");
    }
    close(fd);
    return 0;
}

static void 
read_cu_list(Dwarf_Debug dbg)
{
	Dwarf_Bool is_info = 1;
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
	Dwarf_Half offset_size = 0;
	Dwarf_Half extension_size = 0;
	Dwarf_Sig8 signature ;
	Dwarf_Unsigned typeoffset = 0;
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Error error;
    int cu_number = 0;

    for(;;++cu_number) {
        struct srcfilesdata sf;
        sf.srcfilesres = DW_DLV_ERROR;
        sf.srcfiles = 0;
        sf.srcfilescount = 0;
        Dwarf_Die no_die = 0;
        Dwarf_Die cu_die = 0;
        int res = DW_DLV_ERROR;
		bool noEntry = false;
		// get the info about the next compilation unit header in .debug_info section
        res = dwarf_next_cu_header_c(dbg, is_info, &cu_header_length, &version_stamp, 
			&abbrev_offset, &address_size, &offset_size,
			&extension_size, &signature,   &typeoffset,
		    &next_cu_header, &error);
        noEntry = Error(res, "SEARCHING CU die", true);
		if( noEntry )
			return;

        /* The CU will have a single sibling, a cu_die. */
		// with no_die==NULL as input, this function will get the first die of the cu
        res = dwarf_siblingof_b(dbg, no_die, is_info, &cu_die,&error);
		res = Error( res, "Get the first die within CU", false);
		
		// get the name for CU from the first die of CU
		char *dieName = NULL;
		dwarf_diename(cu_die,&dieName,&error);
		Error(res, "SEARCHING CU die name", true);
		if( dieName )
			printf("===========CU die name:\t%s==============\n", dieName);		

        DepthFirst(dbg,cu_die,0,&sf);
        dwarf_dealloc(dbg,cu_die,DW_DLA_DIE);
        resetsrcfiles(dbg,&sf);
    }
}

// visit into cu in a Depth-First fashion
static void
DepthFirst(Dwarf_Debug dbg, Dwarf_Die in_die,int in_level,
   struct srcfilesdata *sf)
{
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die=in_die;
    Dwarf_Die child = 0;
    Dwarf_Error error;
	Dwarf_Half tag;
   
	for(;;) {
		Dwarf_Die sib_die = 0;

		// focus on dies of interested tags
		res = dwarf_tag(cur_die, &tag,&error);
		Error(res, "SEARCHING die tag", false);
		if( IsGoodTag( tag ) )
		{
			print_die_data(dbg,cur_die,in_level,sf);
			res = dwarf_child(cur_die,&child,&error);
			Error(res, "SEARCHING DFS of CU", true);  
		
			// DFS
		    if(res == DW_DLV_OK) {
		        DepthFirst(dbg,child,in_level+1,sf);
		    }
		}
        
		// WFS
        res = dwarf_siblingof(dbg,cur_die,&sib_die,&error);
		Error(res, "SEARCHING WFS of CU", true);
        if(res == DW_DLV_NO_ENTRY) {
            /* Done at this level. */
            break;
        }

        /* res == DW_DLV_OK */
        if(cur_die != in_die) {
            dwarf_dealloc(dbg,cur_die,DW_DLA_DIE);
        }
        cur_die = sib_die;
        //print_die_data(dbg,cur_die,in_level,sf);
    }
    return;
}
static void
get_addr(Dwarf_Attribute attr,Dwarf_Addr *val)
{
    Dwarf_Error error = 0;
    int res;
    Dwarf_Addr uval = 0;
    res = dwarf_formaddr(attr,&uval,&error);
    if(res == DW_DLV_OK) {
        *val = uval;
        return;
    }
    return;
}
static void
get_number(Dwarf_Attribute attr,Dwarf_Unsigned *val)
{
    Dwarf_Error error = 0;
    int res;
    Dwarf_Signed sval = 0;
    Dwarf_Unsigned uval = 0;
    res = dwarf_formudata(attr,&uval,&error);
    if(res == DW_DLV_OK) {
        *val = uval;
        return;
    }
    res = dwarf_formsdata(attr,&sval,&error);
    if(res == DW_DLV_OK) {
        *val = sval;
        return;
    }
    return;
}
static void
print_subprog(Dwarf_Debug dbg,Dwarf_Die die, int level,
    struct srcfilesdata *sf)
{
    int res;
    Dwarf_Error error = 0;
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Addr lowpc = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Signed attrcount = 0;
    Dwarf_Unsigned i;
    Dwarf_Unsigned filenum = 0;
    Dwarf_Unsigned linenum = 0;
    char *filename = 0;
    res = dwarf_attrlist(die,&attrbuf,&attrcount,&error);
    if(res != DW_DLV_OK) {
        return;
    }
    for(i = 0; i < attrcount ; ++i) {
        Dwarf_Half aform;
        res = dwarf_whatattr(attrbuf[i],&aform,&error);
        if(res == DW_DLV_OK) {
            if(aform == DW_AT_decl_file) {
                get_number(attrbuf[i],&filenum);
                if((filenum > 0) && (sf->srcfilescount > (filenum-1))) {
                    filename = sf->srcfiles[filenum-1];
                }
            }
            if(aform == DW_AT_decl_line) {
                get_number(attrbuf[i],&linenum);
            }
            if(aform == DW_AT_low_pc) {
                get_addr(attrbuf[i],&lowpc);
            }
            if(aform == DW_AT_high_pc) {
                get_addr(attrbuf[i],&highpc);
            }
        }
        dwarf_dealloc(dbg,attrbuf[i],DW_DLA_ATTR);
    }
    if(filenum || linenum) {
        printf("<%3d> file: %" DW_PR_DUu " %s  line %"
            DW_PR_DUu "\n",level,filenum,filename?filename:"",linenum);
    }
    if(lowpc) {
        printf("<%3d> low_pc : 0x%" DW_PR_DUx  "\n",
            level, (Dwarf_Unsigned)lowpc);
    }
    if(highpc) {
        printf("<%3d> high_pc: 0x%" DW_PR_DUx  "\n",
            level, (Dwarf_Unsigned)highpc);
    }
    dwarf_dealloc(dbg,attrbuf,DW_DLA_LIST);
}

static void
print_comp_dir(Dwarf_Debug dbg,Dwarf_Die die,int level, struct srcfilesdata *sf)
{
    int res;
    Dwarf_Error error = 0;
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Signed attrcount = 0;
    Dwarf_Unsigned i;
    res = dwarf_attrlist(die,&attrbuf,&attrcount,&error);
    if(res != DW_DLV_OK) {
        return;
    }
    sf->srcfilesres = dwarf_srcfiles(die,&sf->srcfiles,&sf->srcfilescount, 
        &error);
    for(i = 0; i < attrcount ; ++i) {
        Dwarf_Half aform;
        res = dwarf_whatattr(attrbuf[i],&aform,&error);
        if(res == DW_DLV_OK) {
            if(aform == DW_AT_comp_dir) {
                char *name = 0;
                res = dwarf_formstring(attrbuf[i],&name,&error);
                if(res == DW_DLV_OK) {
                    printf(    "<%3d> compilation directory : \"%s\"\n",
                        level,name);
                }
            }
            if(aform == DW_AT_stmt_list) {
                /* Offset of stmt list for this CU in .debug_line */
            }
        }
        dwarf_dealloc(dbg,attrbuf[i],DW_DLA_ATTR);
    }
    dwarf_dealloc(dbg,attrbuf,DW_DLA_LIST);
}

static void
resetsrcfiles(Dwarf_Debug dbg,struct srcfilesdata *sf)
{
    Dwarf_Signed sri = 0;
    for (sri = 0; sri < sf->srcfilescount; ++sri) {
        dwarf_dealloc(dbg, sf->srcfiles[sri], DW_DLA_STRING);
    }
    dwarf_dealloc(dbg, sf->srcfiles, DW_DLA_LIST);
    sf->srcfilesres = DW_DLV_ERROR;
    sf->srcfiles = 0;
    sf->srcfilescount = 0;
}

static void
print_die_data(Dwarf_Debug dbg, Dwarf_Die print_me,int level,
    struct srcfilesdata *sf)
{
    char *name = 0;
    Dwarf_Error error = 0;
    Dwarf_Half tag = 0;
    const char *tagname = 0;
    int localname = 0;

    int res = dwarf_diename(print_me,&name,&error);
    Error(res, "SEARCH die name", true); 
    if(res == DW_DLV_NO_ENTRY) {
        name = "<no DW_AT_name attr>";
        localname = 1;
    }

    res = dwarf_tag(print_me,&tag,&error);
	Error(res, "SEARCHING die tag", false);

    res = dwarf_get_TAG_name(tag,&tagname);
	Error(res, "SEARCHIGN die tag name", false);
	
	{
		printIndent(level);
		printf("<%d> tag: %d %s  name: \"%s\"\n",level,tag,tagname,name);	
		if( tag == DW_TAG_variable )
		{
			// If external or local variable
			Dwarf_Bool bAttr;
			Dwarf_Bool bForm;		
			enum Dwarf_Form_Class dfc;	
			Dwarf_Attribute attr;
			Dwarf_Half form;
			bool bExternal = false;

			Dwarf_Unsigned constLoc;
			Dwarf_Block *blockLoc;
			
			
			res = dwarf_hasattr( print_me, DW_AT_external, &bAttr, &error);
			Error(res, "SEARCHING external attribute1", true);
			
			if( res == DW_DLV_OK && bAttr == true)
			{
				res = dwarf_attr(print_me, DW_AT_external, &attr, &error);
				Error(res, "SEARCHING external attribute2", true);
				res = dwarf_formflag(attr, &bExternal, &error);
				Error(res, "SEARCHING external attribute3", true);
			}
			
			if( bExternal )
			{
				res = dwarf_hasattr( print_me, DW_AT_location, &bAttr, &error);
				Error(res, "SEARCHING location attribute1", true);
				if( res == DW_DLV_OK && bAttr == true)
				{
					res = dwarf_attr(print_me, DW_AT_location, &attr, &error);
					Error(res, "SEARCHING location attribute2", true);
					res = dwarf_whatform( attr, &form, &error);
					Error(res, "JUDGING form of attribute", true);
					
					dfc = dwarf_get_form_class( 2, DW_AT_location, 4, form );
					if( dfc == DW_FORM_CLASS_CONSTANT )
					{
						res = dwarf_formudata( attr, &constLoc, &error );
						Error(res, "SEARCHING location of constant value", false );
						printf("++++++location:\t%llu\n", constLoc);
												
					}
					else if( dfc == DW_FORM_CLASS_BLOCK )
					{
						res = dwarf_formblock( attr, &blockLoc, &error );
						Error(res, "SEARCHING location of block value", false );
						
						
					}
				}
				
			}
			
		}
	}

    if(!localname) {
        dwarf_dealloc(dbg,name,DW_DLA_STRING);
    }
}

int Error(int res, const char *szInfo, bool bWarn)
{
	if(res == DW_DLV_ERROR) {
            printf("Error for %s \n", szInfo);
            exit(1);
        }
    if(res == DW_DLV_NO_ENTRY) {
        if(!bWarn)
		{
        	printf("no entry for %s \n", szInfo);
        	exit(1);
		}
		return 1;
    }
	return 0;
}

int printIndent(int nTab)
{
	int i = 0;
	for(; i < nTab; ++ i)
		printf("---");
	return 0;
}

bool IsGoodTag(Dwarf_Half tag)
{
	//printf("%d\n", tag);
	if( tag == DW_TAG_base_type ||
		tag == DW_TAG_pointer_type || 
		tag == DW_TAG_const_type ||
		tag == DW_TAG_array_type ||
		tag == DW_TAG_structure_type || 
		tag == DW_TAG_typedef)
		return false;
	return true;
}

