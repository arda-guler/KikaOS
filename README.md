# KikaOS
I took the [Bare Bones](https://wiki.osdev.org/Bare_Bones) x86 C kernel tutorial from [OSDev Wiki](https://wiki.osdev.org/Main_Page) and improved on it by:
- Implementing keyboard input reading,
- Implementing typing on the screen w/ ASCII characters,
- Newline support and screen scrolling.
- Having a permanent terminal title

...which makes it a DOS-like command line rather than a poor 'Hello World'.

That's all. Nothing shiny, nothing fancy.
Next step would be to run other terminal functions by taking user inputs as commands.

As is, the operating system is merely a digital typewriter.

![scr1](https://github.com/arda-guler/KikaOS/blob/master/images/kikaos.jpg)

In the **scripts** folder are the commands you can use to build and run the OS yourselves. It is made to use GRUB as the bootloader. Or, merely take the prebuilt ISO file in the **bin** folder and boot that up.
