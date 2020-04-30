## Unreleased
* Add new option for swapping O/X buttons (#168)
* Add new option for drawing FPS value (#167)
* Fix `RTSP message too long` (#164, fbe3d06)

## 0.7.0
* Support 960x540 resolution (#162, fc7e19c)
* Support GFE 3.20 (97a6a0b)

## 0.6.1
* Fix the stream delay issue (f791b3d); thanks @AlC4pwn to confirm fixing the bug

## 0.6.0
* Update latest moonlight-common-c (cfadd84, ffe8c15)
* Update latest inih (a2e38bb)
* Fix invalid variable initializations
* Apply SPS changes of moonlight-embedded
* Use nerdfont instead VITA system font (7e2b682)
* Use vita2d instead direct framebuffer handling (70330e2)
* Implement poor network indicator (749aaaf)

## 0.5.0
* Support to discover stream server via mDNS (#143)
* Implement frame pacer (#147)
* Update latest enet & moonlight-common-c (904a5d1, 7f63f0d)

## 0.4.1
* Update latest moonlight-common-c (2c9d61c)

## 0.4.0
* Sort app list alphabetically (#127)
* Improve special button settings (#130)
* Implement video cleanup for fixing connection issue (#131)

## 0.3.3
* Update latest moonlight-common-c (8e77710)
* Cherry pick upstream change about gamepad masking (933d700)
* Reduce mismatch between upstream (e2d7910, e3cad393)

## 0.3.2
* New configure option about streaming optimization (d2c974a)
* Fix cannot use 960x544 resolution (653afa6)

## 0.3.1
* Fix cannot connect with new devices (b838278)

## 0.3.0
* Support GFE 3.11
* New configure option about reference frame invalidation (#89, #91)
* Fix unset resolution configure (#90)

## 0.2.0
* Support newer vitasdk (94988ab)
* Support forward error correction (f8631b5, 184bdbe)
* No more build uncompress binary (6fd22e7)

## 0.1.2
* Compress binary (#75)
* Support GFE 3.2 (#76)

## 0.1.1
* Fix connection problem on the GFE 2.2.3 (#66)
* Improve logging datas (#67, #68)
* Support newer vitasdk (#70)

## 0.1.0
* Add new option for store debugging log (#53)
* Fix crash at quit application (#54)
* Add livearea (#62)
* Add new option for mouse acceleration (#63)
* Cleanup codes. (#60)
* Now remove alpha mark. but still lower version :) (#64)

## alpha6
* Reimplement input & config process for the improve stability (#47)
* Fix little memory leaks (#47)
* Fix crash if have too many moonlight supporting games (#47)
* Fix crash at press circle button on the connect menu (#47)
* Fix bug about not detect device model (#50)
* Fix cannot use analog sticks on the VITA 2000 (#50)
* Change priority of input thread for the high bitrate connection (#50)
* Update mapping values & add new mapping file for the PSTV (#51)

## alpha5
* Fix crash at first time (dcd1dc8)
* Improve input packet handle (#24)
* Change connection option (#26)
* Code cleanup for input handling (#27, #30, #31)
* Support L2/R2/L3/R3 Buttons on PSTV (#36)
* Update build toolchains & Fix connection problems under GFE 3.0 (#39)

## alpha4
* Support GUI (#13)
* Support editing config file (#4)
* Support input mapping (#5)
* Add new config options for power saving (#6)
* Turn safe application (#7)

## alpha3
* Support virtual button for L2/R2/L3/R3 using touchscreen (#1, #2)
* Support mouse move and click using touchscreen (#3)

## alpha2
* Support custom settings; screen resolution, framerate and bitrate (d145403)

## alpha1
* Initial release (04a7c1d)
