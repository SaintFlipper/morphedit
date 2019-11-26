Morphedit
=========

Morphedit is an interactive editor for the Emu Morpheus and UltraProteus synthesisers.
It should run on any version of Windows from Windows 95 onwards, and uses standard the Windows MM MIDI API, which still seems to work as of Windows 10.

## Binaries

If you just want to run Morphedit then everything needed should be in the **bin** directory, including a user manual in PDF format.

## Source

The source is obviously in **src**. It's ancient code now, all written in C (not even C++) using the raw Win32 API, and originally built using Microsoft Quick C. At some point it was ported to Visual Studio, but I'm not sure which VS version that was. It appears that as of VS 2017 it doesn't build because various  Windows headers have now been removed from the Windows SDKs.

If you fancy building it on a later Visual Studio version, porting it to other platforms, extending it, or fixing anything you find, feel free to fork the project or take it over. It's really only here for reference and historical interest.

As basic guidance, Morphedit comprises the core morphedit.exe and two DLLs: **morphdll.dll** wraps MIDI I/O and **aucntrl.dll** contains the custom UI widgets used in the editor, for example the slider control. In source terms the DLLs are defined as projects within the workspace, and they are dependencies of the core application.


FAQs
====

The following is a totally out of date FAQ list from the original project host.

**Q**: MorphEdit simply won't connect to the Morpheus / UltraProteus. It gets as far as saying "connecting", sits there awhile, then times out. Why?  
**A**: Hard to say. There are all sorts of reasons why this might happen. First check the hardware connections (both IN and OUT from the PC to the synth). Can any other MIDI program communicate with the synth ? Try both sending notes to the synth (ie. testing MIDI OUT) and recording a SysEx MIDI dump from the synth to another program (presumably a sequencer). Now check that MorphEdit is correctly set up with the MIDI IN/OUT devices in its Options->Preferences dialog. You need a Windows MME (multimedia extensions) compatible MIDI driver for this to work. This is the normal Window MIDI standard, so it really shouldn't be a problem. If it is, then the MorphEdit test keyboard should be able to produce notes on the synth. Now check that the Device ID (also in the Options->Preferences dialog) matches the ID set up from the synth's front panel. If all of these seem OK then it's probably beyond me.

**Q**: I seem to be changing the preset on MorphEdit, but it doesn't make any difference to the actual sound coming out of the synth. Why not?  
**A**: Some versions of the Morpheus/Ultra Proteus don't reflect preset changes until you select the "preset" button (as if you're about to edit using the front panel) on the front panel. Then an external MIDI editor can drive the synth completely. E-mu told me that there is a way to overcome this problem using a MIDI command, but then they lost interest, so I never found out.

**Q**: I've got a preset, and now I want to be able to download it every time I play a particular song. How do I do that?  
**A**: You need to be using a sequencer that can a) import a MIDI song file, and can b) send it out as SysEx (system exclusive) information. Save the edited preset from MorphEdit using the File->Export MIDI file->Single preset command, then import that MIDI file created into your sequencer. This file contains a single preset as SysEx information. What you're trying to do is to tell your sequencer to send this information to the synth at the start of the song.

**Q**: How can I trigger sounds that I'm editing from an external keyboard?  
**A**: It's not really a feature of MorphEdit, but you can usually merge MIDI data streams (ie. the output from the keyboard and the output from MorphEdit) internally within the PC using the MIDI port's own software. Opcode's interfaces have a patchbay application that allows you to patch any input to any output. Remember that you have to connect the synth's MIDI IN to the PC's MIDI OUT, so in order to plug a keyboard into the synth, you'll have to have an separate MIDI in port somewhere.

**Q**: Does MorphEdit work with the Orbit, Phatt, etc etc. since they seem to have the same Z-plane workings?  
**A**: I've got no idea, but I'd be interested to know.

