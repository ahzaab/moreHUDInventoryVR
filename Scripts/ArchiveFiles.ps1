﻿$sourcePath = "$($Env:SkyrimVRPath)\Data"
$destinationDataPath = "$($Env:ModDevPath)\MODS\SkyrimVR\moreHUDInventory\SKSE64\sksevr\moreHUDInventory\Data"
$destinationAS2Path = "$($Env:ModDevPath)\MODS\SkyrimVR\moreHUDInventory\SKSE64\sksevr\moreHUDInventory\AS2"

if (!$(Test-Path "$destinationDataPath\Interface"))
{
    New-Item -ItemType Directory "$destinationDataPath\Interface"
    New-Item -ItemType Directory "$destinationDataPath\Source\Scripts"
}

Copy-Item "$sourcePath\Source\Scripts\ahzMoreHudIE.psc" -Destination "$destinationDataPath\Source\Scripts"
Copy-Item "$sourcePath\Scripts\ahzMoreHudIE.pex" -Destination "$destinationDataPath\Scripts"
Copy-Item "$sourcePath\AHZmoreHUDInventory.esp" -Destination "$destinationDataPath"
Copy-Item "$sourcePath\Interface\AHZmoreHUDInventory.swf" -Destination "$destinationDataPath\Interface"

if ($(Test-Path "$sourcePath\Interface\exported\moreHUDIE"))
{
    if (!$(Test-Path "$destinationDataPath\Interface\exported\moreHUDIE"))
    {
        New-Item -ItemType Directory "$destinationDataPath\Interface\exported\moreHUDIE"
    }
    Copy-Item "$sourcePath\Interface\exported\moreHUDIE\*.*" -Destination "$destinationDataPath\Interface\exported\moreHUDIE"
}


#AS2


Copy-Item "$($Env:ModDevPath)\MODS\SkyrimVR\moreHUDInventory\ScaleForm\src\HUDWidgets\*" -Destination $destinationAS2Path -Exclude .git* -Recurse -Force