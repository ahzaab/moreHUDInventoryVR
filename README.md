
# Description

This Repositiory contains the source for the SKSE64 plugin used by the [moreHUD Inventory Edition](https://www.nexusmods.com/skyrimspecialedition/mods/18619) mod for Skyrim Special Edition.  
The plugin works in conjunction with the [ahzaab/moreHUDInventoryAS2](https://github.com/ahzaab/moreHUDInventoryAS2) Scaleform Elements.  

## How it Works

* The SKSE64 plugin is loaded by [SKSE64](http://skse.silverlock.org/) using the skse64_loader.exe
* The plugin dynammically loads the Scaleform .swf movie clip into the following menus when the menu loads:
  * "InventoryMenu"
  * "Crafting Menu"
  * "ContainerMenu"
  * "BarterMenu"
  * "MagicMenu"

* The plugin registers Scaleform functions used by the ActionScript 2.0 code associated with the [moreHUD Inventory Edition swf file](https://github.com/ahzaab/moreHUDInventoryAS2) 
* The plugin extends the data in the menu's entry list.  When a menu is open, the game transverses all items in the menu.  
  During this time, this plugin extends the entries with information such as Known Enchantments, Number of Effects, etc.

## Installation
The compiled .dll is installed in the Skyrim Data Folder to `Data/SKSE/Plugins`

## Does it need papyrus?
No.  There is no papyrus.  All settings are made in the corresponding ini file.

## Configuration
Starting in version 1.0.15 Mod authors can change parameters and load custom large item card backgrounds as shown [here](https://github.com/ahzaab/moreHUDInventory/tree/master/Data/Interface/exported/moreHUDIE).
