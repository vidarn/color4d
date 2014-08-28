#pragma once
#include "c4d.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"
#include "palettesubdialog.h"

class PaletteDialog : public GeDialog
{
private:
	PaletteSubDialog m_paletteSubDiag;
    Int32 m_id;
public:

	PaletteDialog(Int32 id);
	~PaletteDialog();

    virtual Bool CreateLayout(void);
};

class PaletteCommand : public CommandData
{
	private:
		PaletteDialog dlg;
	public:
        PaletteCommand():dlg(4){}
		virtual Bool Execute(BaseDocument *doc);
		virtual Int32 GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};

class PaletteCommand2 : public CommandData
{
    private:
        PaletteDialog dlg;
    public:
        PaletteCommand2():dlg(5){}
        virtual Bool Execute(BaseDocument *doc);
        virtual Int32 GetState(BaseDocument *doc);
        virtual Bool RestoreLayout(void *secret);
};

class PaletteCommand3 : public CommandData
{
    private:
        PaletteDialog dlg;
    public:
        PaletteCommand3():dlg(6){}
        virtual Bool Execute(BaseDocument *doc);
        virtual Int32 GetState(BaseDocument *doc);
        virtual Bool RestoreLayout(void *secret);
};