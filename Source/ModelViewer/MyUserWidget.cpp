// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "ModelLoader.h"
#include "ue_import/commonutils.h"
#include <Runtime/UMG/Public/Components/CanvasPanelSlot.h>
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <ModelViewer/MeshUtil.h>
#include <ModelViewer/EncodeTransfer.h>
#include <ModelViewer/MyFileBrowserData.h>

#ifdef _WIN32
#include<windows.h>
//打开保存文件对话框
#include<Commdlg.h>

//选择文件夹对话框
#include<Shlobj.h>
#pragma comment(lib,"Shell32.lib")
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#else
#include <filesystem>
using namespace std::__fs::filesystem;
#endif




bool showOpenDialog = true;

std::string defaultPath = "/";
std::string curPath = "/";

FText initialPlotButtonText;
FText initialImportButtonText;

const TCHAR* PlotButtonLabelOnLoading = TEXT("向上");
const TCHAR* ImportButtonLabelOnLoading = TEXT("返回");


bool UMyUserWidget::Initialize()
{
	Super::Initialize();
	ButtonPlot->OnClicked.AddDynamic(this, &UMyUserWidget::OnPlot);
	ButtonImport->OnClicked.AddDynamic(this, &UMyUserWidget::OnImport);
	ListView_FileBrowser->SetVisibility(ESlateVisibility::Collapsed);
	initialPlotButtonText = TextBlock_Plot->GetText();
	initialImportButtonText = TextBlock_Import->GetText();
	//ListView_FileBrowser->OnItemClicked().AddUObject(this,&UMyUserWidget::OnSelectListItem);

	prevFuncName = TEXT("");
	TextDebug->Text = FText::FromString(TEXT(""));
	//MeshUtil::SetTextDebug(TextDebug);
	//UModel3d::SetTextDebug(TextDebug);
	hasInitialPawnData = false;
	scaleOnSetCamera = 1;
	initialRootScale = FVector(1, 1, 1);

#ifdef _WIN32
	defaultPath = "C:/";
	char szFilePath[MAX_PATH + 1];
	szFilePath[0] = 0;
	//GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	//defaultPath = stringReplace(szFilePath, "\\", "/");
	//defaultPath = substring(defaultPath, 0, defaultPath.rfind('/'))+"/";

	GetCurrentDirectoryA(MAX_PATH, szFilePath);
	defaultPath = stringReplace(szFilePath, "\\", "/")+"/";
	//defaultPath = ModelLoader::ToStdStringPath(TEXT("D:/llg/虚拟现实/"));
#endif
#if PLATFORM_ANDROID
	defaultPath = "/sdcard/";
#endif
	curPath = defaultPath;
	return true;
}

void UMyUserWidget::ShowAllAbove() 
{
	TextFunction->SetVisibility(ESlateVisibility::Visible);
	ButtonPlot->SetVisibility(ESlateVisibility::Visible);
	ButtonImport->SetVisibility(ESlateVisibility::Visible);

	//FWidgetTransform t = TextFunction->RenderTransform;
	//FVector2D pos = dynamic_cast<UCanvasPanelSlot*>(TextFunction->Slot)->GetPosition();
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *(pos.ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *(TextFunction->RenderTransform.Translation.ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), TextFunction->RenderTransform.Angle);
}

void UMyUserWidget::ShowImportOnly() 
{
	TextFunction->SetVisibility(ESlateVisibility::Hidden);
	ButtonPlot->SetVisibility(ESlateVisibility::Hidden);
	ButtonImport->SetVisibility(ESlateVisibility::Visible);
}

void UMyUserWidget::BindMesh(AActor* mesh)
{
	if (mesh == nullptr) {
		m_meshComponent = nullptr;
		return;
	}
	UStaticMeshComponent* comp0 = mesh->FindComponentByClass<UStaticMeshComponent>();
	if (comp0 != nullptr) {
		//comp0->SetStaticMesh(nullptr);
		comp0->DestroyComponent();
		comp0 = nullptr;
	}
	UProceduralMeshComponent* comp = mesh->FindComponentByClass<UProceduralMeshComponent>();
	if (comp == nullptr) {
		comp = NewObject<UProceduralMeshComponent>(mesh);
		//mesh->AddInstanceComponent(comp);
		mesh->SetRootComponent(comp);
		comp->RegisterComponent();
	}
	
	initialRootScale = mesh->GetActorRelativeScale3D();
	m_meshComponent = comp;
	if (this->opaqueMaterial == nullptr && m_meshComponent != nullptr) {
		this->opaqueMaterial = m_meshComponent->GetMaterial(0);
	}
}

void UMyUserWidget::BindMaterials(UMaterialInterface* opaqueMaterial0, UMaterialInterface* transparentMaterial0) {
	this->opaqueMaterial = opaqueMaterial0;
	this->transparentMaterial = transparentMaterial0;
	if (this->opaqueMaterial == nullptr && m_meshComponent != nullptr) {
		this->opaqueMaterial = m_meshComponent->GetMaterial(0);
	}
}

bool UMyUserWidget::IsFocused() {
	return TextFunction->HasKeyboardFocus();
}

void UMyUserWidget::OnPlot() {
	if (ListView_FileBrowser->GetVisibility() == ESlateVisibility::Visible) {
		OnSelectListItem(TEXT(".."));
		return;
	}

	FString functionFStr = TextFunction->GetText().ToString();
	ResetCamera();
	if (prevFuncName == functionFStr) {
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("OnPlot(%s)"), *functionFStr);
	if (m_meshComponent == nullptr)return;

	UMaterialInterface* mat = m_meshComponent->GetMaterial(0);
	
	ModelLoader::ClearAttached(m_meshComponent->GetOwner());
	if (model3d != nullptr) {
		//model3d->MarkPendingKill();
	}
	model3d = NewObject<UModel3d>();
	//TextDebug->SetText(FText::FromString(FString::Printf(TEXT("OnPlot(%s)"), *functionFStr)));
	//#if PLATFORM_ANDROID || PLATFORM_IOS
	//model3d->UpdateFunction(TCHAR_TO_UTF8(*functionFStr), 50, 50, 10, 100, 100);
	//#else
	model3d->UpdateFunction(TCHAR_TO_UTF8(*functionFStr), 253, 253, 10, 100, 100);
	//#endif
	MeshUtil::UpdateToUProcdualMesh(m_meshComponent, model3d->m_rawMesh,true,false);
	m_meshComponent->SetMaterial(0, this->opaqueMaterial);
	prevFuncName = functionFStr;

	/*
	* generate gbk character set
	std::string str="\n";
	for (int i = 0xa1; i <= 0xa9; i++) {
		str += "L\"";
		for (int j = 0xa1; j <= 0xfe; j++) {
			str += (char)i;
			str += (char)j;
		}
		str += '\"';
		str += '\n';
	}
	str += ",\n";
	for (int i = 0xb0; i <= 0xf7; i++) {
		str += "L\"";
		for (int j = 0xa1; j <= 0xfe; j++) {
			str += (char)i;
			str += (char)j;
		}
		str += '\"';
		str += '\n';
	}
	str += ",\n";


	const unsigned char defaulti = 0xa1;
	const unsigned char defaultj = 0xa1;
	for (int i = 0x81; i <= 0xa0; i++) {
		str += "L\"";
		for (int j = 0x40; j <= 0xfe; j++) {
			if (j != 0x7f) {
				str += (char)i;
				str += (char)j;
			}
			else {
				str += (char)defaulti;
				str += (char)defaultj;
			}
		}
		str += '\"';
		str += '\n';
	}
	str += ",\n";

	for (int i = 0xaa; i <= 0xfe; i++) {
		str += "L\"";
		for (int j = 0x40; j <= 0xa0; j++) {
			if (j != 0x7f) {
				str += (char)i;
				str += (char)j;
			}
			else {
				str += (char)defaulti;
				str += (char)defaultj;
			}
		}
		str += '\"';
		str += '\n';
	}
	str += ",\n";

	for (int i = 0xa8; i <= 0xa9; i++) {
		str += "L\"";-+E
		for (int j = 0x40; j <= 0xa0; j++) {
			if (j != 0x7f) {
				str += (char)i;
				str += (char)j;
			}
			else {
				str += (char)defaulti;
				str += (char)defaultj;
			}
		}
		str += '\"';
		str += '\n';
	}
	str += ";\n\n";
	UE_LOG(LogTemp, Log, TEXT("%s"), ModelLoader::GBKToUTF16(str).c_str());*/
}

void UMyUserWidget::OpenFileDialog(const FString& DialogTitle, const FString& FileTypes, TArray<FString>& OutFileNames)
{
#ifdef _WIN32
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH];
	// Initialize OPENFILENAME
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(szFile);
	//ofn.lpstrFilter = L"Text files (*.txt)\0*.txt\0\0";
	std::wstring ftstr = *FileTypes;
	std::vector<wchar_t> buf;
	buf.resize(ftstr.size() + 2);
	for (int i = 0; i < buf.size(); i++) {
		if (i < ftstr.size() && ftstr[i] != L'|') {
			buf[i] = ftstr[i];
		}
		else {
			buf[i] = 0;
		}
	}
	ofn.lpstrFilter = &buf[0];
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = *DialogTitle;

	OutFileNames.Empty();
	if (GetOpenFileNameW(&ofn))
	{
		OutFileNames.Emplace(FString(szFile));
	}
#else
#endif 

	
}

std::vector<std::string> extNameFilter;

void UMyUserWidget::SelectFileToListView(const FString& extNames0)
{
	FString extNames = extNames0.Replace(TEXT(","),TEXT(" "));
	extNames = extNames.Replace(TEXT(";"), TEXT(" "));
	extNames = extNames.Replace(TEXT("|"), TEXT(" "));
	extNameFilter.clear();
	if (extNames.Len() > 0) {
		TArray<FString> extNameFilter0;
		extNames.ParseIntoArray(extNameFilter0, TEXT(" "), true);

		for (int i = 0; i < extNameFilter0.Num(); i++) {
			extNameFilter.push_back(TCHAR_TO_UTF8(*(extNameFilter0[i])));
		}
	}

	ListView_FileBrowser->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Plot->SetText(FText::FromString(PlotButtonLabelOnLoading));
	TextBlock_Import->SetText(FText::FromString(ImportButtonLabelOnLoading));

	UpdateFileList();
}

void UMyUserWidget::CloseListView() {
	ListView_FileBrowser->SetVisibility(ESlateVisibility::Collapsed);
	if (model3d == nullptr) {
		TextFunction->SetText(FText::FromString(prevFuncName));
	}
	else {
		TextFunction->SetText(FText::FromString(UTF8_TO_TCHAR(model3d->getFunctionName().c_str())));
	}
	
	TextBlock_Plot->SetText(initialPlotButtonText);
	TextBlock_Import->SetText(initialImportButtonText);
}

void getUTF8FileNameListInDir(std::vector<std::string>& outPaths,const std::string& path0,const std::vector<std::string> &extNameFilter0) {
	outPaths.clear();
	std::string path = stringReplace(path0, "\\", "/");
	/*if (path.size() > 0 && path[path.size() - 1] == '/') {
		path = path.substr(0, path.size() - 1);
	}*/
	std::vector<std::string> dirs;
	std::vector<std::string> filenames;
	directory_iterator list(path);	        //文件入口容器
	for (auto& it : list) {
		std::string name = it.path().filename().u8string();
		//#if _WIN32
		//name = EncodeTransfer::UTF8ToGBK(name);
		//#endif
		if (is_directory(it.path())) {
			name += "/";
			dirs.push_back(name);
		}
		else if(is_regular_file(it.path())){
			if (extNameFilter0.size() == 0) {
				filenames.push_back(name);
			}
			else {
				size_t index = name.rfind('.');
				if (index != std::string::npos) {
					std::string extname = stringLowerCase(name.substr(index+1));
					if (std::find(extNameFilter0.begin(), extNameFilter0.end(), extname) != extNameFilter0.end()) {
						filenames.push_back(name);
					}
				}
				
			}
			
		}
	}
	strNumSort(dirs);
	strNumSort(filenames);
	outPaths.insert(outPaths.end(), dirs.begin(), dirs.end());
	outPaths.insert(outPaths.end(), filenames.begin(), filenames.end());
}


void UMyUserWidget::UpdateFileList()
{
	ListView_FileBrowser->ScrollToTop();
	ListView_FileBrowser->ClearListItems();

	std::vector<std::string> listNames;
	if (curPath=="") {
		for (char c = 'A'; c <= 'Z'; c++) {
			listNames.push_back(c+std::string(":/"));
		}
	}
	else {
		
		getUTF8FileNameListInDir(listNames, curPath, extNameFilter);
		if (curPath != "/") {
			listNames.emplace(listNames.begin(), "..");
		}
	}
	
	for (std::string& strName : listNames) {
		UMyFileBrowserData* data = NewObject<UMyFileBrowserData>();
		data->text = UTF8_TO_TCHAR(strName.c_str());
		ListView_FileBrowser->AddItem(data);
	}
	TextFunction->SetText(FText::FromString(ModelLoader::FromStdStringPath(curPath)));
}


void UMyUserWidget::OnSelectListItem(const FString& itemText)
{
	//UE_LOG(LogTemp, Warning, TEXT("obj.Text==%s"), *itemText);
	std::string name = ModelLoader::ToStdStringPath(itemText);
	std::string path = curPath;
	if (name == ".." || name=="../") {
		if (curPath.size() < 2)return;
		size_t index = path.rfind("/", path.size() - 2);
		if (index == std::string::npos) {
			curPath = "";
		}
		curPath = curPath.substr(0, index + 1);
		UpdateFileList();
	}
	else if (name.size()>1 && name[name.size()-1]=='/') {
		curPath += name;
		UpdateFileList();
	}
	else {
		path = curPath + name;
		CloseListView();
		ImportUrl(ModelLoader::FromStdStringPath(path));
	}
}

void UMyUserWidget::OnImport() {
	if (ListView_FileBrowser->GetVisibility() == ESlateVisibility::Visible) {
		CloseListView();
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("OnImport()"));
	if (m_meshComponent == nullptr) {
		return;
	}

	TArray<FString> fileNames;
	if (showOpenDialog) {
		#ifdef _WIN32
		OpenFileDialog(TEXT("选择模型文件"), TEXT("模型文件(*.3ds;*.obj;*.wrl)|*.3ds;*.obj;*.wrl||"), fileNames);
		if (fileNames.Num() >= 1) {
			ImportUrl(fileNames[0]);
		}
		#else
		SelectFileToListView(TEXT("3ds obj wrl"));
		#endif
	}
	else {
		SelectFileToListView(TEXT("3ds obj wrl"));
	}

}

void UMyUserWidget::ImportUrl(const FString& path) {
	const float angleLimit = 30;
	prevFuncName = TEXT("");
	ResetCamera();
	int dotIndex = 0;
	if (!path.FindLastChar(TCHAR('.'), dotIndex)) {
		dotIndex = -1;
	}
	FString fext = path.Mid(dotIndex + 1).ToLower();
	if (fext == TEXT("wrl") || fext == TEXT("wrz")) {
		ModelLoader::LoadVrmlFileToActor(m_meshComponent->GetOwner(), ModelLoader::ToStdStringPath(path), angleLimit, opaqueMaterial, transparentMaterial, std::bind(&UMyUserWidget::SetCamera, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	else {
		ModelLoader::ClearAttached(m_meshComponent->GetOwner());
		/*UStaticMesh* mesh = ModelLoader::LoadSingleMesh(fileNames[0], angleLimit);
		if (mesh != nullptr) {
			m_meshComponent->SetStaticMesh(mesh);
		}*/
		ModelLoader::LoadSingleMesh(m_meshComponent, path, angleLimit);
	}
}

void UMyUserWidget::ResetCamera() {
	if (m_meshComponent != nullptr) {
		m_meshComponent->GetOwner()->SetActorRelativeScale3D(initialRootScale);
	}
	if (!hasInitialPawnData && GetWorld()->GetFirstPlayerController() != nullptr) {
		initialLocation= GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		initialRotation = GetWorld()->GetFirstPlayerController()->GetControlRotation();
		hasInitialPawnData = true;
		return;
	}
	
	if (GetWorld()->GetFirstPlayerController() != nullptr) {
		GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocationAndRotation(initialLocation, initialRotation);

		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->SetActorRelativeRotation(initialRotation);
		GetWorld()->GetFirstPlayerController()->SetControlRotation(initialRotation);
	}
}

void UMyUserWidget::SetCamera(FVector position, FVector eulerAngles, float fieldOfView) {
	if (m_meshComponent != nullptr) {
		m_meshComponent->GetOwner()->SetActorRelativeScale3D(scaleOnSetCamera*initialRootScale);
	}
	if (m_meshComponent != nullptr && GetWorld()->GetFirstPlayerController() != nullptr) {
		FVector positionAtWorld = m_meshComponent->GetOwner()->GetTransform().TransformPosition(position);
		FQuat rotationAtWorld = m_meshComponent->GetOwner()->GetTransform().TransformRotation(FQuat::MakeFromEuler(eulerAngles));
		GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(positionAtWorld);
		GetWorld()->GetFirstPlayerController()->SetControlRotation(FRotator(rotationAtWorld));
	}
	
}