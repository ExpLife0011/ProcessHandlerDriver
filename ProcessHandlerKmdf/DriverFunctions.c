#include "DriverFunctions.h"

// Irp Complition function
NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}


// Major driver functions to register in DriverEntry

NTSTATUS DispatchReadWrite(IN PDEVICE_OBJECT pDeviceObj, IN PIRP pIrp) {
#ifdef DBG
	PRINT_DEBUG("ReadWrite dispatch routine");
#endif
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	UNREFERENCED_PARAMETER(pDeviceObj);

#ifdef DBG
	PRINT_DEBUG("File ");
	PUNICODE_STRING pUstr = &(pIrpStack->FileObject->FileName);
	DbgPrint("%wZ", pUstr);
#endif
	
	ULONG info = FILE_EXISTS;
	
	// TODO: Check for existing "file"

	if (FlagOn(pIrp->Flags, IRP_READ_OPERATION))
	{
#ifdef DBG
		DbgPrint(" is reading.");
#endif
		
		if (pIrpStack->Parameters.Read.Length < READ_BUFFER_SIZE)
		{
#ifdef DBG
			PRINT_ERROR("Invalid buffer size.");
#endif
			status = STATUS_BUFFER_TOO_SMALL;
		}
		else
		{
			READ_BUFFER_TYPE *buf = NULL;
			if (FlagOn(pIrpStack->FileObject->DeviceObject->Flags, DO_BUFFERED_IO))
			{
#ifdef DBG
				PRINT_DEBUG("Buffered I/O");
#endif
				buf = pIrp->AssociatedIrp.SystemBuffer;
			}
			else
			{
#ifdef DBG
				PRINT_ERROR("Unsupported I/O method");
#endif
				status = STATUS_NOT_SUPPORTED;
			}

			if (NT_SUCCESS(status))
			{
				PDRIVER_EXTENSION_EX pDrvExt = (PDRIVER_EXTENSION_EX)IoGetDriverObjectExtension(pDeviceObj->DriverObject, CLIENT_ID_ADDR);
				if (pDrvExt == NULL)
				{
#ifdef DBG
					PRINT_ERROR("Can't get DriverObjectExtension");
#endif
					*buf = (READ_BUFFER_TYPE)0;
				}
				else
				{
					PLIST_ENTRY targetsList = &(pDrvExt->targetsList);

					if (!IsListEmpty(targetsList))
					{
						PTARGETS_LIST_ENTRY nextTarget = (PTARGETS_LIST_ENTRY)RemoveHeadList(targetsList);

						*buf = nextTarget->data;

						MmFreeNonCachedMemory(nextTarget, sizeof(TARGETS_LIST_ENTRY));
					}
					else
					{
#ifdef DBG
						PRINT_DEBUG("Targets list is empty");
#endif
					}

				}
#ifdef DBG
				PRINT_DEBUG("Value ");
				DbgPrint("%ld is read ", *buf);
#endif
			}
		}
	}
	else
	{
#ifdef DBG
		DbgPrint(" operation not supported.");
#endif
		status = STATUS_NOT_SUPPORTED;
	}

#ifdef DBG
	PRINT_DEBUG("Complite ReadWrite dispatch routine\n");
#endif
	CompleteIrp(pIrp, status, info);
	return status;
}


NTSTATUS DispatchCreateClose(IN PDEVICE_OBJECT pDeviceObj, IN PIRP pIrp) {
#ifdef DBG
	PRINT_DEBUG("CreateClose dispatch routine");
#endif
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	UNREFERENCED_PARAMETER(pDeviceObj);
	
#ifdef DBG
	PRINT_DEBUG("File ");
	PUNICODE_STRING pUstr = &(pIrpStack->FileObject->FileName);
	DbgPrint("%wZ ", pUstr);
#endif

	PDEVICE_EXTENSION pDeviceExt = (PDEVICE_EXTENSION) pDeviceObj->DeviceExtension;

	ULONG info = FILE_EXISTS;
	if (FlagOn(pIrp->Flags, IRP_CREATE_OPERATION))
	{
		PUNICODE_STRING readFileName = &(pDeviceExt->fileName);
		PUNICODE_STRING openingFileName = &(pIrpStack->FileObject->FileName);

		BOOLEAN isReadFile = RtlEqualUnicodeString(openingFileName, readFileName, TRUE);
		if (isReadFile)
		{
			if (!(pDeviceExt->isFileOpen))
			{
				pDeviceExt->isFileOpen = TRUE;
				info = FILE_OPENED;
#ifdef DBG
				DbgPrint(" opened.");
#endif

				// TODO

			}
			else
			{
				status = STATUS_FILE_NOT_AVAILABLE;
#ifdef DBG
				DbgPrint(" can't be opened");
				PRINT_ERROR("File is already opened in another program\n");
#endif
			}
		}
		else
		{
			status = STATUS_FILE_NOT_AVAILABLE;
#ifdef DBG
			DbgPrint(" not found.");
			PRINT_ERROR("Incorrect file name.");
#endif
		}
	}
	else if (FlagOn(pIrp->Flags, IRP_CLOSE_OPERATION))
	{
		pDeviceExt->isFileOpen = FALSE;
		status = STATUS_FILE_CLOSED;
#ifdef DBG
		DbgPrint(" closed.");
#endif
	}

#ifdef DBG
	PRINT_DEBUG("Complite CreateClose dispatch routine\n");
#endif
	CompleteIrp(pIrp, status, info);
	return status;
}


NTSTATUS DeviceControlRoutine(IN PDEVICE_OBJECT pDeviceObj, IN PIRP pIrp) {
#ifdef DBG
	PRINT_DEBUG("DeviceControl dispatch routine");
#endif
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	UNREFERENCED_PARAMETER(pIrpStack);

	UNREFERENCED_PARAMETER(pDeviceObj);

	// TODO

#ifdef DBG
	PRINT_DEBUG("Complite DeviceControl dispatch routine\n");
#endif
	CompleteIrp(pIrp, status, 0);
	return status;
}
