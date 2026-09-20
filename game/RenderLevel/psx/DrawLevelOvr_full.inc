# PSX renderer family shared by the one- and two-player overlays.
# The wrapper supplies the retail symbol, player count, and RDATA symbol.
# NOTE(aalhendi): This is a reconstructed parameterization of retail code, not
# recovered Naughty Dog source.

.include "DrawLevelOvr_common.inc"

# NOTE(aalhendi): Public method macros keep the composition readable;
# their private bodies preserve the literal retail register schedule.

# Private body shared by the dynamic-list and 4x4-list methods.
.macro DRAW_LEVEL_OVR_FULL_LIST_BODY name, edge_source, players
.ifnc \edge_source,projected_edge
.ifnc \edge_source,scratch_grid
	.error "DRAW_LEVEL_OVR_FULL_LIST_BODY received an invalid edge source"
.endif
.endif
.if \players == 1
.Lrender_1p_\name:
	lui	$8,0x8009
	addiu	$8,$8,25604
	sw	$8,100($1)
.endif
.Lrender_2p_\name:
	lw	$3,4($4)
.Ldraw_level_ovr_full_list_\name\()_02:
	beq	$4,$0,.Ldraw_level_ovr_full_list_\name\()_07
	sll	$0,$0,0x0
	lw	$24,28($3)
	lw	$25,24($3)
	lw	$10,200($1)
	lhu	$8,60($24)
	sw	$25,104($1)
	srl	$9,$8,0x3
	andi	$9,$9,0x1fc
	addu	$9,$9,$10
	sw	$9,192($1)
	lw	$9,0($9)
	andi	$8,$8,0x1f
	sw	$8,188($1)
	sw	$9,196($1)
.Ldraw_level_ovr_full_list_\name\()_03:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,6656
.else
	addiu	$8,$2,9984
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Ldraw_level_ovr_full_list_\name\()_04
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Ldraw_level_ovr_full_list_\name\()_04:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Ldraw_level_ovr_full_list_\name\()_05
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	cfc2	$3,$31
	swc2	$17,16($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Ldraw_level_ovr_full_list_\name\()_06
	lw	$20,8($24)
	swc2	$13,32($30)
	swc2	$18,36($30)
	swc2	$14,52($30)
	swc2	$19,56($30)
	swc2	$0,0($30)
	swc2	$1,4($30)
	swc2	$2,20($30)
	swc2	$3,24($30)
	swc2	$4,40($30)
	swc2	$5,44($30)
	lwc2	$0,0($22)
	lwc2	$1,4($22)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$2,0($19)
	lwc2	$3,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$4,0($20)
	lwc2	$5,4($20)
	addu	$25,$1,$0
	sll	$0,$0,0x0
	c2	0x280030
	lw	$15,8($22)
	lw	$16,8($19)
	lw	$17,8($20)
	sw	$15,68($30)
	sw	$16,88($30)
	sw	$17,108($30)
	swc2	$12,72($30)
	cfc2	$3,$31
	swc2	$17,76($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Ldraw_level_ovr_full_list_\name\()_06
	lw	$20,12($24)
	swc2	$13,92($30)
	swc2	$18,96($30)
	swc2	$14,112($30)
	swc2	$19,116($30)
	swc2	$0,60($30)
	swc2	$1,64($30)
	swc2	$2,80($30)
	lhu	$21,16($24)
	swc2	$3,84($30)
	swc2	$4,100($30)
	swc2	$5,104($30)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$0,0($19)
	lwc2	$1,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$2,0($20)
	lwc2	$3,4($20)
	lw	$9,20($24)
	sll	$21,$21,0x4
	addu	$21,$21,$23
	lwc2	$4,0($21)
	lwc2	$5,4($21)
	sw	$9,124($1)
	addiu	$8,$0,8
	c2	0x280030
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	sw	$15,128($30)
	sw	$16,148($30)
	sw	$17,168($30)
	swc2	$12,132($30)
	cfc2	$3,$31
	swc2	$17,136($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Ldraw_level_ovr_full_list_\name\()_06
	swc2	$13,152($30)
	swc2	$18,156($30)
	swc2	$14,172($30)
	swc2	$19,176($30)
	swc2	$0,120($30)
	swc2	$1,124($30)
	swc2	$2,140($30)
	swc2	$3,144($30)
	swc2	$4,160($30)
	swc2	$5,164($30)
	lui	$3,0x50
	ori	$3,$3,0x6478
	jal	.Ldraw_level_ovr_full_list_\name\()_22
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Ldraw_level_ovr_full_list_\name\()_22
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Ldraw_level_ovr_full_list_\name\()_22
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Ldraw_level_ovr_full_list_\name\()_22
	addiu	$10,$0,12
.Ldraw_level_ovr_full_list_\name\()_05:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Ldraw_level_ovr_full_list_\name\()_03
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_02
	lw	$3,4($4)
.Ldraw_level_ovr_full_list_\name\()_06:
	lw	$8,100($1)
	sll	$0,$0,0x0
	sw	$24,0($8)
	addiu	$8,$8,4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_05
	sw	$8,100($1)
.Ldraw_level_ovr_full_list_\name\()_07:
	lw	$8,100($1)
	bgez	$0,.Lrender_full_bucket_continue
	sw	$0,0($8)
.Ldraw_level_ovr_full_list_\name\()_08:
	jr	$31
	sll	$0,$0,0x0
.Ldraw_level_ovr_full_list_\name\()_09:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,120
.Ldraw_level_ovr_full_list_\name\()_10:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,140
.Ldraw_level_ovr_full_list_\name\()_11:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,160
.Ldraw_level_ovr_full_list_\name\()_12:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,60
.Ldraw_level_ovr_full_list_\name\()_13:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,20
.Ldraw_level_ovr_full_list_\name\()_14:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,160
.Ldraw_level_ovr_full_list_\name\()_15:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,80
.Ldraw_level_ovr_full_list_\name\()_16:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_09
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_15
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_16
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_13
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_14
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Ldraw_level_ovr_full_list_\name\()_19
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_12
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_16
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_full_list_\name\()_09
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_14
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_full_list_\name\()_09
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_15
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_12
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_13
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_12
	addiu	$10,$0,12
.Ldraw_level_ovr_full_list_\name\()_17:
	sw	$0,180($30)
	jal	.Ldraw_level_ovr_full_list_\name\()_09
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_full_list_\name\()_10
	addiu	$10,$0,12
	addiu	$3,$0,48
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_full_list_\name\()_11
	addiu	$10,$0,12
	addiu	$3,$0,60
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_12
	addiu	$10,$0,12
.Ldraw_level_ovr_full_list_\name\()_18:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	swc2	$14,12($16)
	jr	$31
	swc2	$19,16($16)
.Ldraw_level_ovr_full_list_\name\()_19:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_full_list_\name\()_20
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_full_list_\name\()_20:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_full_list_\name\()_08
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_full_list_\name\()_08
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_full_list_\name\()_21
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_full_list_\name\()_21:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Ldraw_level_ovr_full_list_\name\()_08
	sw	$3,112($1)
	sw	$31,60($25)
.if \players == 1
	addiu	$8,$1,804
	beq	$8,$30,.Ldraw_level_ovr_full_list_\name\()_39
.else
	addiu	$3,$1,804
	beq	$3,$30,.Ldraw_level_ovr_full_list_\name\()_39
.endif
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_34
.else
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_36
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Ldraw_level_ovr_full_list_\name\()_22:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_full_list_\name\()_23
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_full_list_\name\()_23:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_full_list_\name\()_08
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_full_list_\name\()_08
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_full_list_\name\()_24
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_full_list_\name\()_24:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Ldraw_level_ovr_full_list_\name\()_08
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Ldraw_level_ovr_full_list_\name\()_25
	addu	$28,$11,$0
	addu	$28,$12,$0
.Ldraw_level_ovr_full_list_\name\()_25:
	subu	$10,$28,$13
	bgtz	$10,.Ldraw_level_ovr_full_list_\name\()_26
	addu	$3,$3,$24
	addu	$28,$13,$0
.Ldraw_level_ovr_full_list_\name\()_26:
	subu	$10,$28,$14
	bgtz	$10,.Ldraw_level_ovr_full_list_\name\()_27
	lw	$3,28($3)
	addu	$28,$14,$0
.Ldraw_level_ovr_full_list_\name\()_27:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Ldraw_level_ovr_full_list_\name\()_28
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Ldraw_level_ovr_full_list_\name\()_28:
.if \players == 1
	bgez	$28,.Ldraw_level_ovr_full_list_\name\()_29
.else
	bgez	$28,.Ldraw_level_ovr_full_list_\name\()_30
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Ldraw_level_ovr_full_list_\name\()_29:
	lw	$8,28($1)
.endif
.Ldraw_level_ovr_full_list_\name\()_30:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Ldraw_level_ovr_full_list_\name\()_31
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Ldraw_level_ovr_full_list_\name\()_31
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Ldraw_level_ovr_full_list_\name\()_31
	sll	$0,$0,0x0
	bltz	$9,.Ldraw_level_ovr_full_list_\name\()_31
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Ldraw_level_ovr_full_list_\name\()_31:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Ldraw_level_ovr_full_list_\name\()_32
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Ldraw_level_ovr_full_list_\name\()_33
.else
	bltz	$10,.Ldraw_level_ovr_full_list_\name\()_35
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_33
.else
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_35
.endif
	sh	$9,6($22)
.Ldraw_level_ovr_full_list_\name\()_32:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Ldraw_level_ovr_full_list_\name\()_33
.else
	bltz	$10,.Ldraw_level_ovr_full_list_\name\()_35
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Ldraw_level_ovr_full_list_\name\()_33:
	lw	$3,36($1)
.Ldraw_level_ovr_full_list_\name\()_34:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Ldraw_level_ovr_full_list_\name\()_35:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Ldraw_level_ovr_full_list_\name\()_36:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Ldraw_level_ovr_full_list_\name\()_38
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Ldraw_level_ovr_full_list_\name\()_18
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Ldraw_level_ovr_full_list_\name\()_18
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Ldraw_level_ovr_full_list_\name\()_18
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Ldraw_level_ovr_full_list_\name\()_18
	addiu	$16,$30,160
.ifc \edge_source,projected_edge
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
.else
	addiu	$13,$30,80
	addiu	$14,$30,160
	addiu	$15,$30,80
.endif
	jal	.Ldraw_level_ovr_full_list_\name\()_18
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Ldraw_level_ovr_full_list_\name\()_37)
	addiu	$8,$8,%lo(.Ldraw_level_ovr_full_list_\name\()_37)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Ldraw_level_ovr_full_list_\name\()_37:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Ldraw_level_ovr_full_list_\name\()_38:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Ldraw_level_ovr_full_list_\name\()_39:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Ldraw_level_ovr_full_list_\name\()_42
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Ldraw_level_ovr_full_list_\name\()_41
	lw	$3,156($1)
	lui	$10,%hi(.Ldraw_level_ovr_full_list_\name\()_17)
	addiu	$10,$10,%lo(.Ldraw_level_ovr_full_list_\name\()_17)
	bne	$3,$10,.Ldraw_level_ovr_full_list_\name\()_42
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Ldraw_level_ovr_full_list_\name\()_40
	lw	$10,132($1)
	addiu	$8,$8,192
.Ldraw_level_ovr_full_list_\name\()_40:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Ldraw_level_ovr_full_list_\name\()_41:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Ldraw_level_ovr_full_list_\name\()_42:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_41
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_full_list_\name\()_43
	lui	$3,0x3600
	lui	$3,0x3400
.Ldraw_level_ovr_full_list_\name\()_43:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Ldraw_level_ovr_full_list_\name\()_45
	lui	$10,0x900
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_full_list_\name\()_44
	lui	$3,0x3e00
	lui	$3,0x3c00
.Ldraw_level_ovr_full_list_\name\()_44:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Ldraw_level_ovr_full_list_\name\()_45:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.endm

.macro DRAW_LEVEL_OVR_DYNAMIC_LIST players
	DRAW_LEVEL_OVR_FULL_LIST_BODY dynamic_list, projected_edge, \players
.endm

.macro DRAW_LEVEL_OVR_QUAD_4X4_LIST players
	DRAW_LEVEL_OVR_FULL_LIST_BODY quad_4x4_list, scratch_grid, \players
.endm

.macro DRAW_LEVEL_OVR_FULL_CLIP_DISPATCHES rdata
	DRAW_LEVEL_OVR_CLIP_DISPATCH_BODY 1p, \rdata, 1284, .Lrender_full_9b8c
.endm

.macro DRAW_LEVEL_OVR_FULL_WATER_BSP_LIST players
	DRAW_LEVEL_OVR_WATER_BSP_LIST_BODY full, \players, .Lrender_full_terminal_return, .Lrender_full_bucket_continue, 0
.endm

.macro DRAW_LEVEL_OVR_FULL_WATER_RENDERED players
	DRAW_LEVEL_OVR_WATER_RENDERED_BODY full, \players, .Lrender_full_terminal_return, .Lrender_full_bucket_continue, 0
.endm

# Private body shared by the dynamic-rendered and 4x4-rendered methods.
.macro DRAW_LEVEL_OVR_FULL_RENDERED_BODY name, edge_source, players
.ifnc \edge_source,projected_edge
.ifnc \edge_source,scratch_grid
	.error "DRAW_LEVEL_OVR_FULL_RENDERED_BODY received an invalid edge source"
.endif
.endif
.Lrender_1p_\name:
.Lrender_2p_\name:
	lw	$24,0($4)
.Ldraw_level_ovr_full_rendered_\name\()_02:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,6656
.else
	addiu	$8,$2,9984
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	addiu	$30,$1,436
	beq	$24,$0,.Lrender_full_bucket_continue
	DRAW_LEVEL_OVR_PREPARE_RENDERED_QUAD
	jal	.Ldraw_level_ovr_full_rendered_\name\()_18
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Ldraw_level_ovr_full_rendered_\name\()_18
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Ldraw_level_ovr_full_rendered_\name\()_18
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Ldraw_level_ovr_full_rendered_\name\()_18
	addiu	$10,$0,12
	lw	$24,4($4)
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_02
	addiu	$4,$4,4
.Ldraw_level_ovr_full_rendered_\name\()_03:
	jr	$31
	sll	$0,$0,0x0
.Ldraw_level_ovr_full_rendered_\name\()_04:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,120
.Ldraw_level_ovr_full_rendered_\name\()_05:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,140
.Ldraw_level_ovr_full_rendered_\name\()_06:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,160
.Ldraw_level_ovr_full_rendered_\name\()_07:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,60
.Ldraw_level_ovr_full_rendered_\name\()_08:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,20
.Ldraw_level_ovr_full_rendered_\name\()_09:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,160
.Ldraw_level_ovr_full_rendered_\name\()_10:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,80
.Ldraw_level_ovr_full_rendered_\name\()_11:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_04
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_10
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_11
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_08
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_09
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Ldraw_level_ovr_full_rendered_\name\()_14
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_07
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_11
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_full_rendered_\name\()_04
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_09
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_full_rendered_\name\()_04
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_10
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_07
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_08
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_07
	addiu	$10,$0,12
.Ldraw_level_ovr_full_rendered_\name\()_12:
	sw	$0,180($30)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_04
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_05
	addiu	$10,$0,12
	addiu	$3,$0,48
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_06
	addiu	$10,$0,12
	addiu	$3,$0,60
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_07
	addiu	$10,$0,12
.Ldraw_level_ovr_full_rendered_\name\()_13:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	lw	$11,92($1)
	mfc2	$3,$19
	swc2	$14,12($16)
	sw	$3,16($16)
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	sb	$8,18($16)
	sll	$3,$3,0x1
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	jr	$31
	sb	$8,19($16)
.Ldraw_level_ovr_full_rendered_\name\()_14:
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_03
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_full_rendered_\name\()_17
	addiu	$3,$0,12
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_full_rendered_\name\()_15
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_full_rendered_\name\()_15:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_full_rendered_\name\()_03
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_full_rendered_\name\()_03
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_16
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_full_rendered_\name\()_16:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_03
.Ldraw_level_ovr_full_rendered_\name\()_17:
	sw	$3,112($1)
	sw	$31,60($25)
.if \players == 1
	addiu	$8,$1,804
	beq	$8,$30,.Ldraw_level_ovr_full_rendered_\name\()_36
.else
	addiu	$3,$1,804
	beq	$3,$30,.Ldraw_level_ovr_full_rendered_\name\()_36
.endif
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_31
.else
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_33
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Ldraw_level_ovr_full_rendered_\name\()_18:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_03
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_full_rendered_\name\()_21
	addiu	$3,$0,12
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_full_rendered_\name\()_19
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_full_rendered_\name\()_19:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_full_rendered_\name\()_03
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_full_rendered_\name\()_03
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_20
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_full_rendered_\name\()_20:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_03
.Ldraw_level_ovr_full_rendered_\name\()_21:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	srl	$9,$9,0x1f
	sll	$9,$9,0x1f
	srl	$10,$10,0x1f
	sll	$10,$10,0x1e
	or	$10,$10,$9
	sw	$10,128($1)
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Ldraw_level_ovr_full_rendered_\name\()_22
	addu	$28,$11,$0
	addu	$28,$12,$0
.Ldraw_level_ovr_full_rendered_\name\()_22:
	subu	$10,$28,$13
	bgtz	$10,.Ldraw_level_ovr_full_rendered_\name\()_23
	addu	$3,$3,$24
	addu	$28,$13,$0
.Ldraw_level_ovr_full_rendered_\name\()_23:
	subu	$10,$28,$14
	bgtz	$10,.Ldraw_level_ovr_full_rendered_\name\()_24
	lw	$3,28($3)
	addu	$28,$14,$0
.Ldraw_level_ovr_full_rendered_\name\()_24:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Ldraw_level_ovr_full_rendered_\name\()_25
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Ldraw_level_ovr_full_rendered_\name\()_25:
.if \players == 1
	bgez	$28,.Ldraw_level_ovr_full_rendered_\name\()_26
.else
	bgez	$28,.Ldraw_level_ovr_full_rendered_\name\()_27
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Ldraw_level_ovr_full_rendered_\name\()_26:
	lw	$8,28($1)
.endif
.Ldraw_level_ovr_full_rendered_\name\()_27:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Ldraw_level_ovr_full_rendered_\name\()_28
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Ldraw_level_ovr_full_rendered_\name\()_28
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Ldraw_level_ovr_full_rendered_\name\()_28
	sll	$0,$0,0x0
	bltz	$9,.Ldraw_level_ovr_full_rendered_\name\()_28
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Ldraw_level_ovr_full_rendered_\name\()_28:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_29
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_30
.else
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_32
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_30
.else
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_32
.endif
	sh	$9,6($22)
.Ldraw_level_ovr_full_rendered_\name\()_29:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_30
.else
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_32
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Ldraw_level_ovr_full_rendered_\name\()_30:
	lw	$3,36($1)
.Ldraw_level_ovr_full_rendered_\name\()_31:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Ldraw_level_ovr_full_rendered_\name\()_32:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Ldraw_level_ovr_full_rendered_\name\()_33:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Ldraw_level_ovr_full_rendered_\name\()_35
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_13
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Ldraw_level_ovr_full_rendered_\name\()_13
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Ldraw_level_ovr_full_rendered_\name\()_13
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Ldraw_level_ovr_full_rendered_\name\()_13
	addiu	$16,$30,160
.ifc \edge_source,projected_edge
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
.else
	addiu	$13,$30,80
	addiu	$14,$30,160
	addiu	$15,$30,80
.endif
	jal	.Ldraw_level_ovr_full_rendered_\name\()_13
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Ldraw_level_ovr_full_rendered_\name\()_34)
	addiu	$8,$8,%lo(.Ldraw_level_ovr_full_rendered_\name\()_34)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Ldraw_level_ovr_full_rendered_\name\()_34:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Ldraw_level_ovr_full_rendered_\name\()_35:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Ldraw_level_ovr_full_rendered_\name\()_36:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Ldraw_level_ovr_full_rendered_\name\()_39
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Ldraw_level_ovr_full_rendered_\name\()_38
	lw	$3,156($1)
	lui	$10,%hi(.Ldraw_level_ovr_full_rendered_\name\()_12)
	addiu	$10,$10,%lo(.Ldraw_level_ovr_full_rendered_\name\()_12)
	bne	$3,$10,.Ldraw_level_ovr_full_rendered_\name\()_39
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Ldraw_level_ovr_full_rendered_\name\()_37
	lw	$10,132($1)
	addiu	$8,$8,192
.Ldraw_level_ovr_full_rendered_\name\()_37:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Ldraw_level_ovr_full_rendered_\name\()_38:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Ldraw_level_ovr_full_rendered_\name\()_39:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_38
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	and	$3,$15,$16
	and	$3,$3,$17
	bltz	$3,.Ldraw_level_ovr_full_rendered_\name\()_03
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	or	$15,$15,$16
	or	$15,$15,$17
	bltz	$15,.Ldraw_level_ovr_full_rendered_\name\()_43
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_full_rendered_\name\()_40
	lui	$3,0x3600
	lui	$3,0x3400
.Ldraw_level_ovr_full_rendered_\name\()_40:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Ldraw_level_ovr_full_rendered_\name\()_42
	lui	$10,0x900
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_full_rendered_\name\()_03
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_full_rendered_\name\()_45
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_full_rendered_\name\()_41
	lui	$3,0x3e00
	lui	$3,0x3c00
.Ldraw_level_ovr_full_rendered_\name\()_41:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Ldraw_level_ovr_full_rendered_\name\()_42:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.Ldraw_level_ovr_full_rendered_\name\()_43:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($19)
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_44
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($20)
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_44
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($21)
	lw	$31,88($1)
	bgez	$10,.Ldraw_level_ovr_full_rendered_\name\()_03
.Ldraw_level_ovr_full_rendered_\name\()_44:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,36
	lw	$31,88($1)
	addiu	$15,$15,48
	jr	$31
	sw	$15,16($1)
.Ldraw_level_ovr_full_rendered_\name\()_45:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($19)
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_46
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($20)
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_46
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($21)
	bltz	$10,.Ldraw_level_ovr_full_rendered_\name\()_46
	lh	$3,0($22)
	lh	$8,2($22)
	jal	.Ldraw_level_ovr_full_rendered_\name\()_47
	lh	$9,4($22)
	lw	$31,88($1)
	bgez	$10,.Ldraw_level_ovr_full_rendered_\name\()_03
.Ldraw_level_ovr_full_rendered_\name\()_46:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	ori	$3,$3,0x1
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,36
	addu	$13,$22,$0
	jal	.Ldraw_level_ovr_full_rendered_\name\()_51
	addiu	$16,$15,48
	lw	$31,88($1)
	addiu	$15,$15,60
	jr	$31
	sw	$15,16($1)
.Ldraw_level_ovr_full_rendered_\name\()_47:
	subu	$3,$3,$15
	bgez	$3,.Ldraw_level_ovr_full_rendered_\name\()_48
	subu	$8,$8,$16
	neg	$3,$3
.Ldraw_level_ovr_full_rendered_\name\()_48:
	addiu	$11,$3,-256
	bgez	$8,.Ldraw_level_ovr_full_rendered_\name\()_49
	subu	$9,$9,$17
	neg	$8,$8
.Ldraw_level_ovr_full_rendered_\name\()_49:
	addiu	$12,$8,-384
	bgez	$9,.Ldraw_level_ovr_full_rendered_\name\()_50
	sll	$0,$0,0x0
	neg	$9,$9
.Ldraw_level_ovr_full_rendered_\name\()_50:
	addiu	$13,$9,-256
	and	$11,$11,$12
	jr	$31
	and	$10,$11,$13
.Ldraw_level_ovr_full_rendered_\name\()_51:
	lb	$3,19($13)
	lw	$8,0($13)
	lw	$9,4($13)
	lw	$10,8($13)
	sll	$3,$3,0x18
	or	$10,$10,$3
	sw	$8,0($16)
	sw	$9,4($16)
	jr	$31
	sw	$10,8($16)
.endm

.macro DRAW_LEVEL_OVR_DYNAMIC_RENDERED players
	DRAW_LEVEL_OVR_FULL_RENDERED_BODY dynamic_rendered, projected_edge, \players
.endm

.macro DRAW_LEVEL_OVR_QUAD_4X4_RENDERED players
	DRAW_LEVEL_OVR_FULL_RENDERED_BODY quad_4x4_rendered, scratch_grid, \players
.endm

.macro DRAW_LEVEL_OVR_FULL symbol, players, rdata
.if \players != 1
.if \players != 2
	.error "DRAW_LEVEL_OVR_FULL requires one or two players"
.endif
.endif
.set noreorder
.set noat
.set nomacro
.section .text
.align 2
.globl \symbol
.ent \symbol
\symbol:
.if \players == 1
	.set .Lrender_full_terminal_return,.Lrender_full_1p_01f8
	.set .Lrender_full_bucket_continue,.Lrender_full_1p_01cc
	.set .Lrender_full_dynamic_wrapper,.Lrender_full_1p_074c
.else
	.set .Lrender_full_terminal_return,.Lrender_full_2p_0208
	.set .Lrender_full_bucket_continue,.Lrender_full_2p_0344
	.set .Lrender_full_dynamic_wrapper,.Lrender_full_0750
.endif
	DRAW_LEVEL_OVR_SAVE_CONTEXT
	lw	$8,76($29)
	lw	$9,80($29)
.if \players == 2
	lw	$10,84($29)
.endif
	lw	$2,12($7)
	lw	$3,8($7)
	lw	$29,20($7)
	sw	$3,48($1)
	lw	$3,12($6)
	beq	$8,$0,.Lrender_full_terminal_return
.if \players == 1
	sw	$8,200($1)
.else
	sw	$8,204($1)
.endif
.if \players == 2
	beq	$9,$0,.Lrender_full_terminal_return
	sw	$9,208($1)
.endif
	beq	$3,$0,.Lrender_full_terminal_return
.if \players == 1
	sw	$9,136($1)
.else
	sw	$10,136($1)
	sw	$5,220($1)
	addiu	$5,$5,272
	sw	$5,224($1)
.endif
	lui	$10,0x8008
	addiu	$10,$10,16936
	lw	$3,0($10)
.if \players == 1
	sw	$5,220($1)
.else
	lw	$8,4($10)
.endif
.if \players == 1
	sw	$3,16($1)
.else
	sw	$3,0($1)
.endif
.if \players == 2
	sw	$8,4($1)
	addiu	$13,$0,548
	addiu	$11,$1,436
.Lrender_full_2p_009c:
	sw	$0,0($11)
	addiu	$11,$11,4
	bgtz	$13,.Lrender_full_2p_009c
	addiu	$13,$13,-4
	addiu	$13,$0,23
	lui	$11,%hi(\rdata + 88)
	addiu	$11,$11,%lo(\rdata + 88)
	addiu	$12,$1,236
.Lrender_full_2p_00bc:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lrender_full_2p_00bc
	addiu	$13,$13,-1
	lw	$23,16($6)
	sw	$4,96($1)
	addiu	$3,$0,40
.Lrender_full_2p_00e0:
	bltz	$3,.Lrender_full_2p_01d4
	addu	$8,$3,$4
	lw	$4,0($8)
	sw	$3,52($1)
	beq	$4,$0,.Lrender_full_2p_0138
	sll	$0,$0,0x0
	jal	.Lrender_full_2p_0248
	sll	$0,$0,0x0
	lw	$10,204($1)
	lw	$9,0($1)
	lui	$8,0x8009
	addiu	$8,$8,25604
	jal	.Lrender_full_2p_02a4
	lw	$5,220($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lrender_full_2p_012c)
	addiu	$9,$9,%lo(.Lrender_full_2p_012c)
	jr	$8
	sw	$9,84($1)
.Lrender_full_2p_012c:
	lw	$9,16($1)
	bgez	$0,.Lrender_full_2p_0164
	sw	$9,0($1)
.Lrender_full_2p_0138:
	lw	$4,96($1)
	lw	$3,52($1)
	lui	$9,0x8009
	addiu	$9,$9,25604
	addu	$8,$3,$4
	lw	$4,48($8)
	sw	$0,0($9)
	beq	$4,$0,.Lrender_full_2p_01b8
	lui	$31,%hi(.Lrender_full_2p_0180)
	bgez	$0,.Lrender_full_2p_0248
	ori	$31,$31,%lo(.Lrender_full_2p_0180)
.Lrender_full_2p_0164:
	lw	$4,96($1)
	lw	$3,52($1)
	sll	$0,$0,0x0
	addu	$8,$3,$4
	lw	$4,48($8)
	sll	$0,$0,0x0
	beq	$4,$0,.Lrender_full_2p_01b8
.Lrender_full_2p_0180:
	lw	$10,208($1)
	lw	$9,4($1)
	lui	$8,0x8009
	addiu	$8,$8,25860
	jal	.Lrender_full_2p_02a4
	lw	$5,224($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lrender_full_2p_01ac)
	addiu	$9,$9,%lo(.Lrender_full_2p_01ac)
	jr	$8
	sw	$9,84($1)
.Lrender_full_2p_01ac:
	lw	$9,16($1)
	bgez	$0,.Lrender_full_2p_01c4
	sw	$9,4($1)
.Lrender_full_2p_01b8:
	lui	$3,0x8009
	addiu	$3,$3,25860
	sw	$0,0($3)
.Lrender_full_2p_01c4:
	lw	$4,96($1)
	lw	$3,52($1)
	bgez	$0,.Lrender_full_2p_00e0
	addiu	$3,$3,-4
.Lrender_full_2p_01d4:
	jal	.Ldraw_level_ovr_copy_clip_record_jump_table
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 0, .Lrender_full_9ad4, 0
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 1, .Lrender_full_9ad4, 0
.Lrender_full_2p_0208:
	sw	$29,20($7)
	lw	$29,56($1)
	sw	$2,12($7)
	lw	$31,56($29)
	lw	$30,52($29)
	lw	$28,48($29)
	lw	$23,44($29)
	lw	$22,40($29)
	lw	$21,36($29)
	lw	$20,32($29)
	lw	$19,28($29)
	lw	$18,24($29)
	lw	$17,20($29)
	lw	$16,16($29)
	jr	$31
	addiu	$29,$29,60
.Lrender_full_2p_0248:
	lui	$8,%hi(\rdata + 0)
	addiu	$8,$8,%lo(\rdata + 0)
	addu	$8,$8,$3
	lw	$10,0($8)
	lui	$8,%hi(\rdata + 44)
	addiu	$8,$8,%lo(\rdata + 44)
	addu	$8,$8,$3
	lw	$8,0($8)
.Lrender_full_2p_0268:
	lw	$13,0($10)
	lw	$11,4($10)
	lw	$12,8($10)
	beq	$13,$0,.Lrender_full_2p_029c
	addu	$12,$12,$1
.Lrender_full_2p_027c:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lrender_full_2p_027c
	addiu	$13,$13,-1
	bgez	$0,.Lrender_full_2p_0268
	addi	$10,$10,12
.Lrender_full_2p_029c:
	jr	$31
	sw	$8,116($1)
.Lrender_full_2p_02a4:
	sw	$8,100($1)
	sw	$9,16($1)
	sw	$10,200($1)
.endif
	lw	$11,40($5)
	lw	$12,44($5)
	lw	$13,48($5)
	lw	$14,52($5)
	lw	$15,56($5)
	lw	$16,60($5)
	lw	$17,64($5)
	lw	$18,68($5)
	ctc2	$11,$0
	ctc2	$12,$1
	ctc2	$13,$2
	ctc2	$14,$3
	ctc2	$15,$4
	ctc2	$16,$5
	ctc2	$17,$6
	ctc2	$18,$7
	lh	$16,12($5)
	lh	$17,14($5)
	lh	$18,16($5)
	sh	$16,408($1)
	sh	$17,410($1)
	sh	$18,412($1)
	lh	$3,32($5)
	lh	$8,34($5)
	lw	$9,24($5)
	sll	$3,$3,0xf
	sll	$8,$8,0xf
	ctc2	$3,$24
	ctc2	$8,$25
	ctc2	$9,$26
	sra	$9,$9,0x1
	addiu	$9,$9,1
	sw	$9,92($1)
	lw	$3,32($5)
	lw	$5,244($5)
.if \players == 2
	jr	$31
.endif
	sw	$3,108($1)
.if \players == 1
	addiu	$13,$0,548
	addiu	$11,$1,436
.Lrender_full_1p_0110:
	sw	$0,0($11)
	addiu	$11,$11,4
	bgtz	$13,.Lrender_full_1p_0110
	addiu	$13,$13,-4
	addiu	$13,$0,23
	lui	$11,%hi(\rdata + 88)
	addiu	$11,$11,%lo(\rdata + 88)
	addiu	$12,$1,236
.Lrender_full_1p_0130:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lrender_full_1p_0130
	addiu	$13,$13,-1
	lw	$23,16($6)
	sw	$4,96($1)
	addiu	$3,$0,40
.Lrender_full_1p_0154:
	bltz	$3,.Lrender_full_1p_01dc
	addu	$8,$3,$4
	sw	$3,52($1)
	lw	$4,0($8)
	lui	$8,%hi(\rdata + 0)
	addiu	$8,$8,%lo(\rdata + 0)
	addu	$8,$8,$3
	beq	$4,$0,.Lrender_full_1p_01c0
	lw	$10,0($8)
	lui	$8,%hi(\rdata + 44)
	addiu	$8,$8,%lo(\rdata + 44)
	addu	$8,$8,$3
	lw	$8,0($8)
.Lrender_full_1p_0188:
	lw	$13,0($10)
	lw	$11,4($10)
	lw	$12,8($10)
	beq	$13,$0,.Lrender_full_1p_01bc
	addu	$12,$12,$1
.Lrender_full_1p_019c:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lrender_full_1p_019c
	addiu	$13,$13,-1
	bgez	$0,.Lrender_full_1p_0188
	addi	$10,$10,12
.Lrender_full_1p_01bc:
	jr	$8
.Lrender_full_1p_01c0:
	lui	$3,0x8009
	addiu	$3,$3,25604
	sw	$0,0($3)
.Lrender_full_1p_01cc:
	lw	$4,96($1)
	lw	$3,52($1)
	bgez	$0,.Lrender_full_1p_0154
	addiu	$3,$3,-4
.Lrender_full_1p_01dc:
	jal	.Ldraw_level_ovr_copy_clip_record_jump_table
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 0, .Lrender_full_9ad4, 16
.Lrender_full_1p_01f8:
	sw	$29,20($7)
	lw	$29,56($1)
	sw	$2,12($7)
	lw	$31,56($29)
	lw	$30,52($29)
	lw	$28,48($29)
	lw	$23,44($29)
	lw	$22,40($29)
	lw	$21,36($29)
	lw	$20,32($29)
	lw	$19,28($29)
	lw	$18,24($29)
	lw	$17,20($29)
	lw	$16,16($29)
.else
.Lrender_full_2p_0344:
	lw	$31,84($1)
	sll	$0,$0,0x0
.endif
	jr	$31
.if \players == 1
	addiu	$29,$29,60
.else
	sll	$0,$0,0x0
.endif
# Full-dynamic bucket entry.
.Lrender_1p_full_dynamic:
.Lrender_2p_full_dynamic:
	lw	$3,4($4)
.Lrender_full_023c:
	beq	$4,$0,.Lrender_full_bucket_continue
	sll	$0,$0,0x0
	lw	$24,28($3)
	lw	$25,24($3)
	lw	$10,200($1)
	lhu	$8,60($24)
	sw	$25,104($1)
	srl	$9,$8,0x3
	andi	$9,$9,0x1fc
	addu	$9,$9,$10
	sw	$9,192($1)
	lw	$9,0($9)
	andi	$8,$8,0x1f
	sw	$8,188($1)
	sw	$9,196($1)
.Lrender_full_0278:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,9984
.else
	addiu	$8,$2,13312
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Lrender_full_02ac
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Lrender_full_02ac:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Lrender_full_0724
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	swc2	$17,16($30)
	swc2	$13,32($30)
	swc2	$18,36($30)
	swc2	$14,52($30)
	swc2	$19,56($30)
	swc2	$0,0($30)
	swc2	$1,4($30)
	swc2	$2,20($30)
	swc2	$3,24($30)
	swc2	$4,40($30)
	swc2	$5,44($30)
	lw	$3,20($24)
	lw	$18,8($22)
	lwc2	$0,0($22)
	lwc2	$1,4($22)
	addiu	$19,$30,0
	addiu	$20,$30,20
	c2	0x180001
	addiu	$21,$30,40
	addiu	$22,$30,60
	sw	$3,124($1)
	sw	$18,68($30)
	swc2	$14,72($30)
	swc2	$19,76($30)
	swc2	$0,60($30)
	swc2	$1,64($30)
	lw	$3,64($24)
	sw	$0,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	lw	$10,8($3)
	sw	$8,416($1)
	sw	$9,420($1)
	sw	$10,424($1)
	sh	$8,6($19)
	sh	$9,6($20)
	sh	$10,6($21)
	srl	$10,$10,0x10
	sh	$10,6($22)
	mfc2	$11,$16
	mfc2	$12,$17
	mfc2	$13,$18
	mfc2	$14,$19
	subu	$3,$11,$12
	bgtz	$3,.Lrender_full_03f0
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_03f0:
	subu	$3,$28,$13
	bgtz	$3,.Lrender_full_0400
	lb	$9,124($1)
	addu	$28,$13,$0
.Lrender_full_0400:
	subu	$3,$28,$14
	bgtz	$3,.Lrender_full_0410
	srl	$28,$28,0x6
	srl	$28,$14,0x6
.Lrender_full_0410:
	addu	$28,$28,$9
.if \players == 1
	lw	$9,20($1)
.endif
	sll	$28,$28,0x2
	addu	$28,$28,$5
.if \players == 1
	subu	$17,$11,$9
	subu	$18,$12,$9
	subu	$15,$13,$9
	subu	$16,$14,$9
.else
	addiu	$17,$11,-6592
	addiu	$18,$12,-6592
	addiu	$15,$13,-6592
	addiu	$16,$14,-6592
.endif
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_067c
	addu	$25,$1,$0
	addu	$11,$17,$1
	lw	$11,328($11)
	lw	$20,8($24)
	lw	$22,12($24)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$0,0($19)
	lwc2	$1,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$2,0($20)
	lwc2	$3,4($20)
	andi	$21,$22,0xffff
	sll	$21,$21,0x4
	addu	$21,$21,$23
	lwc2	$4,0($21)
	lwc2	$5,4($21)
	srl	$22,$22,0x10
	sll	$22,$22,0x4
	c2	0x280030
	addu	$22,$22,$23
	sw	$19,160($1)
	sw	$20,164($1)
	sw	$21,168($1)
	sw	$22,172($1)
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	sw	$15,88($30)
	sw	$16,108($30)
	sw	$17,128($30)
	lh	$19,16($24)
	swc2	$12,92($30)
	swc2	$17,96($30)
	swc2	$13,112($30)
	swc2	$18,116($30)
	swc2	$14,132($30)
	swc2	$19,136($30)
	swc2	$0,80($30)
	swc2	$1,84($30)
	swc2	$2,100($30)
	swc2	$3,104($30)
	swc2	$4,120($30)
	swc2	$5,124($30)
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$0,0($22)
	lwc2	$1,4($22)
	lwc2	$2,0($19)
	lwc2	$3,4($19)
	lui	$3,%hi(.Lrender_full_0724)
	addiu	$3,$3,%lo(.Lrender_full_0724)
	c2	0x280030
	sw	$3,60($25)
	addiu	$25,$25,4
	sw	$19,176($1)
	lw	$18,8($22)
	lw	$16,8($19)
	sw	$18,148($30)
	sw	$16,168($30)
	swc2	$12,152($30)
	swc2	$17,156($30)
	swc2	$13,172($30)
	swc2	$18,176($30)
	swc2	$0,140($30)
	swc2	$1,144($30)
	swc2	$2,160($30)
	swc2	$3,164($30)
	lw	$13,160($1)
	addiu	$15,$30,80
	addiu	$16,$30,0
	jal	.Lrender_full_dynamic_wrapper
	addiu	$18,$30,20
	lw	$13,164($1)
	addiu	$15,$30,100
	addiu	$16,$30,0
	jal	.Lrender_full_dynamic_wrapper
	addiu	$18,$30,40
	lw	$13,168($1)
	addiu	$15,$30,120
	addiu	$16,$30,20
	jal	.Lrender_full_dynamic_wrapper
	addiu	$18,$30,40
	lw	$13,172($1)
	addiu	$15,$30,140
	addiu	$16,$30,20
	jal	.Lrender_full_dynamic_wrapper
	addiu	$18,$30,60
	lw	$13,176($1)
	addiu	$15,$30,160
	addiu	$16,$30,40
	jal	.Lrender_full_dynamic_wrapper
	addiu	$18,$30,60
	lbu	$3,6($30)
	lbu	$8,26($30)
	lbu	$9,46($30)
	lbu	$10,66($30)
	addu	$15,$3,$8
	addu	$16,$3,$9
	addu	$17,$8,$9
	addu	$18,$8,$10
	addu	$3,$9,$10
	sra	$15,$15,0x1
	sra	$16,$16,0x1
	sra	$17,$17,0x1
	sra	$18,$18,0x1
	sra	$3,$3,0x1
	sb	$15,86($30)
	sb	$16,106($30)
	sb	$17,126($30)
	sb	$18,146($30)
	sb	$3,166($30)
	lbu	$3,7($30)
	lbu	$8,27($30)
	lbu	$9,47($30)
	lbu	$10,67($30)
	addu	$15,$3,$8
	addu	$16,$3,$9
	addu	$17,$8,$9
	addu	$18,$8,$10
	addu	$3,$9,$10
	sra	$15,$15,0x1
	sra	$16,$16,0x1
	sra	$17,$17,0x1
	sra	$18,$18,0x1
	sra	$3,$3,0x1
	sb	$15,87($30)
	sb	$16,107($30)
	sb	$17,127($30)
	sb	$18,147($30)
	jr	$11
	sb	$3,167($30)
.Lrender_full_067c:
	lw	$8,108($1)
	lw	$9,124($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	neg	$8,$8
	or	$8,$8,$9
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_0724
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_0724
	mfc2	$3,$24
	srl	$8,$8,0x1f
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_0724
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	sll	$0,$0,0x0
.Lrender_full_0724:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Lrender_full_0278
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Lrender_full_023c
	lw	$3,4($4)
.Lrender_full_0744:
	jr	$31
	sll	$0,$0,0x0
.if \players == 1
.Lrender_full_1p_074c:
	lw	$10,44($1)
.endif
.Lrender_full_0750:
	lw	$3,16($15)
	lh	$8,0($16)
.if \players == 1
	subu	$3,$3,$10
.else
	addiu	$3,$3,-5760
.endif
	sll	$3,$3,0x2
	bltz	$3,.Lrender_full_0870
	addiu	$10,$3,-4096
	lh	$9,0($18)
	mtc2	$3,$8
	addu	$3,$8,$9
	sra	$3,$3,0x1
	lh	$8,2($16)
	lh	$9,2($18)
	sh	$3,180($1)
	addu	$3,$8,$9
	sra	$3,$3,0x1
	lh	$8,4($16)
	lh	$9,4($18)
	sh	$3,182($1)
	addu	$3,$8,$9
	sra	$3,$3,0x1
	sw	$3,184($1)
	bgez	$10,.Lrender_full_0844
	lw	$9,12($13)
	lwc2	$6,8($15)
	sll	$3,$9,0x4
	srl	$8,$3,0x10
	srl	$9,$3,0x8
	andi	$8,$8,0xff0
	andi	$9,$9,0xff0
	andi	$3,$3,0xff0
	ctc2	$8,$23
	ctc2	$9,$22
	ctc2	$3,$21
	sll	$0,$0,0x0
	sll	$0,$0,0x0
	c2	0x780010
	mfc2	$9,$22
	lh	$3,0($15)
	lh	$8,180($1)
	mtc2	$3,$25
	subu	$8,$8,$3
	mtc2	$8,$9
	lh	$3,2($15)
	lh	$8,182($1)
	mtc2	$3,$26
	subu	$8,$8,$3
	mtc2	$8,$10
	lh	$3,4($15)
	lh	$8,184($1)
	mtc2	$3,$27
	subu	$8,$8,$3
	mtc2	$8,$11
	sll	$0,$0,0x0
	sll	$0,$0,0x0
	c2	0x1a8003e
	mfc2	$3,$25
	mfc2	$8,$26
	mfc2	$10,$27
	sh	$3,180($1)
	sh	$8,182($1)
	sw	$10,184($1)
.Lrender_full_0844:
	lw	$3,180($1)
	lw	$8,184($1)
	mtc2	$3,$0
	mtc2	$8,$1
	sw	$3,0($15)
	sh	$8,4($15)
	c2	0x180001
	sw	$9,8($15)
	swc2	$14,12($15)
	jr	$31
	swc2	$19,16($15)
.Lrender_full_0870:
	jr	$31
	sll	$0,$0,0x0
.Lrender_full_0878:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,120
.Lrender_full_088c:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,140
.Lrender_full_08a0:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,160
.Lrender_full_08b4:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,60
.Lrender_full_08c8:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,20
.Lrender_full_08dc:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,160
.Lrender_full_08f0:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,80
.Lrender_full_0904:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Lrender_full_0c04
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Lrender_full_0c04
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_0c04
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_0878
	addiu	$10,$0,12
	jal	.Lrender_full_088c
	addiu	$10,$0,12
	jal	.Lrender_full_08f0
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_0904
	addiu	$10,$0,12
	jal	.Lrender_full_08c8
	addiu	$10,$0,12
	jal	.Lrender_full_08a0
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08dc
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Lrender_full_0c04
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_0c04
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08b4
	addiu	$10,$0,12
	jal	.Lrender_full_0904
	addiu	$10,$0,12
	jal	.Lrender_full_08a0
	addiu	$10,$0,4
	jal	.Lrender_full_0878
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_088c
	addiu	$10,$0,12
	jal	.Lrender_full_08dc
	addiu	$10,$0,12
	jal	.Lrender_full_088c
	addiu	$10,$0,4
	jal	.Lrender_full_0878
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08a0
	addiu	$10,$0,12
	jal	.Lrender_full_08f0
	addiu	$10,$0,12
	jal	.Lrender_full_08a0
	addiu	$10,$0,8
	jal	.Lrender_full_088c
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08b4
	addiu	$10,$0,12
	jal	.Lrender_full_08c8
	addiu	$10,$0,12
	jal	.Lrender_full_088c
	addiu	$10,$0,8
	jal	.Lrender_full_08a0
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08b4
	addiu	$10,$0,12
	addiu	$3,$1,436
	bne	$3,$30,.Lrender_full_0ad8
	lui	$3,0x50
	ori	$3,$3,0x6478
	addiu	$8,$0,8
	jal	.Lrender_full_0cec
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lrender_full_0cec
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lrender_full_0cec
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lrender_full_0cec
	addiu	$10,$0,12
	bgez	$0,.Lrender_full_0724
	sll	$0,$0,0x0
.Lrender_full_0ad8:
	sw	$0,180($30)
	jal	.Lrender_full_0878
	addiu	$10,$0,12
	addiu	$3,$0,4
	sw	$3,180($30)
	jal	.Lrender_full_088c
	addiu	$10,$0,12
	addiu	$3,$0,8
	sw	$3,180($30)
	jal	.Lrender_full_08a0
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_08b4
	addiu	$10,$0,12
.Lrender_full_0b1c:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	swc2	$14,12($16)
	jr	$31
	swc2	$19,16($16)
.Lrender_full_0c04:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_0c58
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_0c58:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_0744
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_0744
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_0c98
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_0c98:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lrender_full_0744
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,436
	bne	$8,$30,.Lrender_full_0cc8
.Lrender_full_0cc8:
	addiu	$8,$1,804
	beq	$8,$30,.Lrender_full_1008
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 1
	bgez	$0,.Lrender_full_1p_0f2c
.else
	bgez	$0,.Lrender_full_0f30
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$3,$0,768
.endif
.Lrender_full_0cec:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_0da0
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_0da0:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_0744
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_0744
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_0de0
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_0de0:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_0744
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Lrender_full_0e34
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_0e34:
	subu	$10,$28,$13
	bgtz	$10,.Lrender_full_0e44
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lrender_full_0e44:
	subu	$10,$28,$14
	bgtz	$10,.Lrender_full_0e54
	lw	$3,28($3)
	addu	$28,$14,$0
.Lrender_full_0e54:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lrender_full_0e70
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lrender_full_0e70:
.if \players == 1
	bgez	$28,.Lrender_full_1p_0e7c
.else
	bgez	$28,.Lrender_full_0e80
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Lrender_full_1p_0e7c:
	lw	$8,28($1)
.endif
.Lrender_full_0e80:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Lrender_full_0ec0
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_0ec0
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_0ec0
	sll	$0,$0,0x0
	bltz	$9,.Lrender_full_0ec0
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lrender_full_0ec0:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lrender_full_0f08
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_0f28
.else
	bltz	$10,.Lrender_full_2p_1030
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Lrender_full_1p_0f28
.else
	bgez	$0,.Lrender_full_2p_1030
.endif
	sh	$9,6($22)
.Lrender_full_0f08:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_0f28
.else
	bltz	$10,.Lrender_full_2p_1030
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Lrender_full_1p_0f28:
	lw	$3,36($1)
.Lrender_full_1p_0f2c:
	sll	$0,$0,0x0
.else
.Lrender_full_2p_1030:
	addiu	$3,$0,1536
.endif
.Lrender_full_0f30:
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_1008
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Lrender_full_0b1c
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Lrender_full_0b1c
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Lrender_full_0b1c
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Lrender_full_0b1c
	addiu	$16,$30,160
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
	jal	.Lrender_full_0b1c
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_0ff8)
	addiu	$8,$8,%lo(.Lrender_full_0ff8)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_0ff8:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lrender_full_1008:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
	addu	$19,$20,$0
	addu	$20,$22,$0
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	lw	$11,12($19)
	lw	$12,12($20)
	lw	$13,12($21)
	lh	$19,6($19)
	lh	$20,6($20)
	lh	$21,6($21)
	sh	$19,416($1)
	sh	$20,420($1)
	sh	$21,424($1)
	lw	$19,416($1)
	lw	$20,420($1)
	lw	$21,424($1)
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_107c
	lui	$3,0x3600
	lui	$3,0x3400
.Lrender_full_107c:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Lrender_full_1150
	lui	$10,0x900
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	lw	$18,8($22)
	lw	$11,12($19)
	lw	$12,12($20)
	lw	$13,12($21)
	lw	$14,12($22)
	lh	$19,6($19)
	lh	$20,6($20)
	lh	$21,6($21)
	sh	$19,416($1)
	sh	$20,420($1)
	lh	$20,6($22)
	sh	$21,424($1)
	sh	$20,426($1)
	lw	$19,416($1)
	lw	$20,420($1)
	lw	$21,424($1)
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_1110
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lrender_full_1110:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lrender_full_1150:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
# Water BSP-list bucket entry.
	DRAW_LEVEL_OVR_FULL_WATER_BSP_LIST \players
# Previously rendered water bucket entry.
	DRAW_LEVEL_OVR_FULL_WATER_RENDERED \players
.if \players == 1
# Ground 4x1 BSP-list bucket entry.
.Lrender_1p_ground_4x1_list:
	lui	$8,0x8009
	addiu	$8,$8,25604
	sw	$8,100($1)
.endif
.Lrender_2p_ground_4x1_list:
	lw	$3,4($4)
.Lrender_full_29fc:
	beq	$4,$0,.Lrender_full_2cf4
	sll	$0,$0,0x0
	lw	$24,28($3)
	lw	$25,24($3)
	lw	$10,200($1)
	lhu	$8,60($24)
	sw	$25,104($1)
	srl	$9,$8,0x3
	andi	$9,$9,0x1fc
	addu	$9,$9,$10
	sw	$9,192($1)
	lw	$9,0($9)
	andi	$8,$8,0x1f
	sw	$8,188($1)
	sw	$9,196($1)
.Lrender_full_2a38:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,4160
.else
	addiu	$8,$2,7488
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Lrender_full_2a6c
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Lrender_full_2a6c:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Lrender_full_2cbc
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	cfc2	$3,$31
	swc2	$17,16($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_2cdc
	lw	$20,8($24)
	swc2	$13,32($30)
	swc2	$18,36($30)
	swc2	$14,52($30)
	swc2	$19,56($30)
	swc2	$0,0($30)
	swc2	$1,4($30)
	swc2	$2,20($30)
	swc2	$3,24($30)
	swc2	$4,40($30)
	swc2	$5,44($30)
	lwc2	$0,0($22)
	lwc2	$1,4($22)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$2,0($19)
	lwc2	$3,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$4,0($20)
	lwc2	$5,4($20)
	addu	$25,$1,$0
	sll	$0,$0,0x0
	c2	0x280030
	lw	$15,8($22)
	lw	$16,8($19)
	lw	$17,8($20)
	sw	$15,68($30)
	sw	$16,88($30)
	sw	$17,108($30)
	swc2	$12,72($30)
	cfc2	$3,$31
	swc2	$17,76($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_2cdc
	lw	$20,12($24)
	swc2	$13,92($30)
	swc2	$18,96($30)
	swc2	$14,112($30)
	swc2	$19,116($30)
	swc2	$0,60($30)
	swc2	$1,64($30)
	swc2	$2,80($30)
	lhu	$21,16($24)
	swc2	$3,84($30)
	swc2	$4,100($30)
	swc2	$5,104($30)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$0,0($19)
	lwc2	$1,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$2,0($20)
	lwc2	$3,4($20)
	lw	$9,20($24)
	sll	$21,$21,0x4
	addu	$21,$21,$23
	lwc2	$4,0($21)
	lwc2	$5,4($21)
	sw	$9,124($1)
	addiu	$8,$0,8
	c2	0x280030
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	sw	$15,128($30)
	sw	$16,148($30)
	sw	$17,168($30)
	swc2	$12,132($30)
	cfc2	$3,$31
	swc2	$17,136($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_2cdc
	swc2	$13,152($30)
	swc2	$18,156($30)
	swc2	$14,172($30)
	swc2	$19,176($30)
	swc2	$0,120($30)
	swc2	$1,124($30)
	swc2	$2,140($30)
	swc2	$3,144($30)
	swc2	$4,160($30)
	swc2	$5,164($30)
	lui	$3,0x50
	ori	$3,$3,0x6478
	jal	.Lrender_full_2fb4
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lrender_full_2fb4
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lrender_full_2fb4
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lrender_full_2fb4
	addiu	$10,$0,12
.Lrender_full_2cbc:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Lrender_full_2a38
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Lrender_full_29fc
	lw	$3,4($4)
.Lrender_full_2cdc:
	lw	$8,100($1)
	sll	$0,$0,0x0
	sw	$24,0($8)
	addiu	$8,$8,4
	bgez	$0,.Lrender_full_2cbc
	sw	$8,100($1)
.Lrender_full_2cf4:
	lw	$8,100($1)
	bgez	$0,.Lrender_full_bucket_continue
	sw	$0,0($8)
.Lrender_full_2d00:
	jr	$31
	sll	$0,$0,0x0
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_2ed4
	addiu	$10,$0,12
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,80
	addiu	$22,$30,40
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_2ed4
	addiu	$10,$0,4
	addiu	$19,$30,20
	addiu	$20,$30,160
	addiu	$21,$30,0
	addiu	$22,$30,40
	jal	.Lrender_full_2ed4
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_2ed4
	addiu	$10,$0,8
.Lrender_full_2d78:
	sw	$0,180($30)
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_2ed4
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_2ed4
	addiu	$10,$0,12
.Lrender_full_2dbc:
	lh	$11,0($13)
	lh	$12,0($14)
	sh	$11,0($15)
	sh	$12,0($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,0($18)
	lh	$11,2($13)
	lh	$12,2($14)
	sh	$11,2($15)
	sh	$12,2($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,2($18)
	lh	$11,4($13)
	lh	$12,4($14)
	sh	$11,4($15)
	sh	$12,4($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	lwc2	$0,0($18)
	mtc2	$3,$1
	sh	$3,4($18)
	lbu	$11,8($13)
	c2	0x180001
	lbu	$12,8($14)
	sb	$11,8($15)
	sb	$12,8($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,8($18)
	lbu	$11,9($13)
	lbu	$12,9($14)
	sb	$11,9($15)
	sb	$12,9($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,9($18)
	lbu	$11,10($13)
	lbu	$12,10($14)
	sb	$11,10($15)
	sb	$12,10($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,10($18)
	lbu	$11,6($13)
	lbu	$12,6($14)
	sb	$11,6($15)
	sb	$12,6($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,6($18)
	lbu	$11,7($13)
	lbu	$12,7($14)
	sb	$11,7($15)
	sb	$12,7($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,7($18)
	lw	$3,12($13)
	lw	$8,16($13)
	sw	$3,12($15)
	sw	$8,16($15)
	lw	$3,12($14)
	lw	$8,16($14)
	sw	$3,12($16)
	sw	$8,16($16)
	swc2	$14,12($18)
	jr	$31
	swc2	$19,16($18)
.Lrender_full_2ed4:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_2f28
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_2f28:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_2d00
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_2d00
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_2f68
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_2f68:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lrender_full_2d00
	sw	$3,112($1)
	sw	$31,60($25)
.if \players == 1
	addiu	$8,$1,804
	beq	$8,$30,.Lrender_full_32b8
.else
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_32b8
.endif
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Lrender_full_1p_31f4
.else
	bgez	$0,.Lrender_full_3208
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Lrender_full_2fb4:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_3068
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_3068:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_2d00
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_2d00
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_30a8
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_30a8:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_2d00
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Lrender_full_30fc
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_30fc:
	subu	$10,$28,$13
	bgtz	$10,.Lrender_full_310c
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lrender_full_310c:
	subu	$10,$28,$14
	bgtz	$10,.Lrender_full_311c
	lw	$3,28($3)
	addu	$28,$14,$0
.Lrender_full_311c:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lrender_full_3138
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lrender_full_3138:
.if \players == 1
	bgez	$28,.Lrender_full_1p_3144
.else
	bgez	$28,.Lrender_full_3148
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Lrender_full_1p_3144:
	lw	$8,28($1)
.endif
.Lrender_full_3148:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Lrender_full_3188
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_3188
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_3188
	sll	$0,$0,0x0
	bltz	$9,.Lrender_full_3188
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lrender_full_3188:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lrender_full_31d0
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_31f0
.else
	bltz	$10,.Lrender_full_2p_32d4
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Lrender_full_1p_31f0
.else
	bgez	$0,.Lrender_full_2p_32d4
.endif
	sh	$9,6($22)
.Lrender_full_31d0:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_31f0
.else
	bltz	$10,.Lrender_full_2p_32d4
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Lrender_full_1p_31f0:
	lw	$3,36($1)
.Lrender_full_1p_31f4:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Lrender_full_2p_32d4:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Lrender_full_3208:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_329c
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	addiu	$16,$30,20
	jal	.Lrender_full_2dbc
	addiu	$18,$30,80
	addu	$13,$21,$0
	addu	$14,$22,$0
	addiu	$15,$30,40
	addiu	$16,$30,60
	jal	.Lrender_full_2dbc
	addiu	$18,$30,160
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_328c)
	addiu	$8,$8,%lo(.Lrender_full_328c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_328c:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lrender_full_329c:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lrender_full_32b8:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lrender_full_335c
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lrender_full_3328
	lw	$3,156($1)
	lui	$10,%hi(.Lrender_full_2d78)
	addiu	$10,$10,%lo(.Lrender_full_2d78)
	bne	$3,$10,.Lrender_full_335c
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lrender_full_32f8
	lw	$10,132($1)
	addiu	$8,$8,48
.Lrender_full_32f8:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Lrender_full_3328:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Lrender_full_335c:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lrender_full_3328
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_33c8
	lui	$3,0x3600
	lui	$3,0x3400
.Lrender_full_33c8:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Lrender_full_349c
	lui	$10,0x900
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_345c
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lrender_full_345c:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lrender_full_349c:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
# Previously rendered ground 4x1 bucket entry.
.Lrender_1p_ground_4x1_rendered:
.Lrender_2p_ground_4x1_rendered:
	lw	$24,0($4)
.Lrender_full_34c4:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,4160
.else
	addiu	$8,$2,7488
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	addiu	$30,$1,436
	beq	$24,$0,.Lrender_full_bucket_continue
	DRAW_LEVEL_OVR_PREPARE_RENDERED_QUAD
	jal	.Lrender_full_3b38
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lrender_full_3b38
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lrender_full_3b38
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lrender_full_3b38
	addiu	$10,$0,12
	lw	$24,4($4)
	bgez	$0,.Lrender_full_34c4
	addiu	$4,$4,4
.Lrender_full_381c:
	jr	$31
	sll	$0,$0,0x0
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_3a14
	addiu	$10,$0,12
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,80
	addiu	$22,$30,40
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_3a14
	addiu	$10,$0,4
	addiu	$19,$30,20
	addiu	$20,$30,160
	addiu	$21,$30,0
	addiu	$22,$30,40
	jal	.Lrender_full_3a14
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_3a14
	addiu	$10,$0,8
.Lrender_full_3894:
	sw	$0,180($30)
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_3a14
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_3a14
	addiu	$10,$0,12
.Lrender_full_38d8:
	lh	$11,0($13)
	lh	$12,0($14)
	sh	$11,0($15)
	sh	$12,0($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,0($18)
	lh	$11,2($13)
	lh	$12,2($14)
	sh	$11,2($15)
	sh	$12,2($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,2($18)
	lh	$11,4($13)
	lh	$12,4($14)
	sh	$11,4($15)
	sh	$12,4($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	lwc2	$0,0($18)
	mtc2	$3,$1
	sh	$3,4($18)
	lbu	$11,8($13)
	c2	0x180001
	lbu	$12,8($14)
	sb	$11,8($15)
	sb	$12,8($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,8($18)
	lbu	$11,9($13)
	lbu	$12,9($14)
	sb	$11,9($15)
	sb	$12,9($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,9($18)
	lbu	$11,10($13)
	lbu	$12,10($14)
	sb	$11,10($15)
	sb	$12,10($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,10($18)
	lbu	$11,6($13)
	lbu	$12,6($14)
	sb	$11,6($15)
	sb	$12,6($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,6($18)
	lbu	$11,7($13)
	lbu	$12,7($14)
	sb	$11,7($15)
	sb	$12,7($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,7($18)
	lw	$3,12($13)
	lw	$8,16($13)
	sw	$3,12($15)
	sw	$8,16($15)
	lw	$3,12($14)
	lw	$8,16($14)
	sw	$3,12($16)
	sw	$8,16($16)
	lw	$11,92($1)
	mfc2	$3,$19
	swc2	$14,12($18)
	sw	$3,16($18)
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	sb	$8,18($18)
	sll	$3,$3,0x1
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	jr	$31
	sb	$8,19($18)
.Lrender_full_3a14:
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_381c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_3b0c
	addiu	$3,$0,12
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_3aac
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_3aac:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_381c
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_381c
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_3aec
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_3aec:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lrender_full_381c
.Lrender_full_3b0c:
	sw	$3,112($1)
	sw	$31,60($25)
.if \players == 1
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_3e98
.else
	addiu	$8,$1,804
	beq	$8,$30,.Lrender_full_3e98
.endif
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Lrender_full_1p_3dd4
.else
	bgez	$0,.Lrender_full_3de8
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Lrender_full_3b38:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_381c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_3c88
	addiu	$3,$0,12
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_3c30
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_3c30:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_381c
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_381c
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_3c70
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_3c70:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_381c
.Lrender_full_3c88:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	srl	$9,$9,0x1f
	sll	$9,$9,0x1f
	srl	$10,$10,0x1f
	sll	$10,$10,0x1e
	or	$10,$10,$9
	sw	$10,128($1)
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Lrender_full_3cdc
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_3cdc:
	subu	$10,$28,$13
	bgtz	$10,.Lrender_full_3cec
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lrender_full_3cec:
	subu	$10,$28,$14
	bgtz	$10,.Lrender_full_3cfc
	lw	$3,28($3)
	addu	$28,$14,$0
.Lrender_full_3cfc:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lrender_full_3d18
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lrender_full_3d18:
.if \players == 1
	bgez	$28,.Lrender_full_1p_3d24
.else
	bgez	$28,.Lrender_full_3d28
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Lrender_full_1p_3d24:
	lw	$8,28($1)
.endif
.Lrender_full_3d28:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Lrender_full_3d68
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_3d68
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_3d68
	sll	$0,$0,0x0
	bltz	$9,.Lrender_full_3d68
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lrender_full_3d68:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lrender_full_3db0
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_3dd0
.else
	bltz	$10,.Lrender_full_2p_3eac
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Lrender_full_1p_3dd0
.else
	bgez	$0,.Lrender_full_2p_3eac
.endif
	sh	$9,6($22)
.Lrender_full_3db0:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_3dd0
.else
	bltz	$10,.Lrender_full_2p_3eac
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Lrender_full_1p_3dd0:
	lw	$3,36($1)
.Lrender_full_1p_3dd4:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Lrender_full_2p_3eac:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Lrender_full_3de8:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_3e7c
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	addiu	$16,$30,20
	jal	.Lrender_full_38d8
	addiu	$18,$30,80
	addu	$13,$21,$0
	addu	$14,$22,$0
	addiu	$15,$30,40
	addiu	$16,$30,60
	jal	.Lrender_full_38d8
	addiu	$18,$30,160
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_3e6c)
	addiu	$8,$8,%lo(.Lrender_full_3e6c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_3e6c:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lrender_full_3e7c:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lrender_full_3e98:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lrender_full_3f3c
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lrender_full_3f08
	lw	$3,156($1)
	lui	$10,%hi(.Lrender_full_3894)
	addiu	$10,$10,%lo(.Lrender_full_3894)
	bne	$3,$10,.Lrender_full_3f3c
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lrender_full_3ed8
	lw	$10,132($1)
	addiu	$8,$8,48
.Lrender_full_3ed8:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Lrender_full_3f08:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Lrender_full_3f3c:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lrender_full_3f08
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	and	$3,$15,$16
	and	$3,$3,$17
	bltz	$3,.Lrender_full_381c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	or	$15,$15,$16
	or	$15,$15,$17
	bltz	$15,.Lrender_full_4110
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_3fd8
	lui	$3,0x3600
	lui	$3,0x3400
.Lrender_full_3fd8:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Lrender_full_40ec
	lui	$10,0x900
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_381c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_41b4
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_40ac
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lrender_full_40ac:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lrender_full_40ec:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.Lrender_full_4110:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lrender_full_427c
	lh	$9,4($19)
	bltz	$10,.Lrender_full_4160
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lrender_full_427c
	lh	$9,4($20)
	bltz	$10,.Lrender_full_4160
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lrender_full_427c
	lh	$9,4($21)
	lw	$31,88($1)
	bgez	$10,.Lrender_full_381c
.Lrender_full_4160:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,36
	lw	$31,88($1)
	addiu	$15,$15,48
	jr	$31
	sw	$15,16($1)
.Lrender_full_41b4:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lrender_full_427c
	lh	$9,4($19)
	bltz	$10,.Lrender_full_4218
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lrender_full_427c
	lh	$9,4($20)
	bltz	$10,.Lrender_full_4218
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lrender_full_427c
	lh	$9,4($21)
	bltz	$10,.Lrender_full_4218
	lh	$3,0($22)
	lh	$8,2($22)
	jal	.Lrender_full_427c
	lh	$9,4($22)
	lw	$31,88($1)
	bgez	$10,.Lrender_full_381c
.Lrender_full_4218:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	ori	$3,$3,0x1
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,36
	addu	$13,$22,$0
	jal	.Lrender_full_42bc
	addiu	$16,$15,48
	lw	$31,88($1)
	addiu	$15,$15,60
	jr	$31
	sw	$15,16($1)
.Lrender_full_427c:
	subu	$3,$3,$15
	bgez	$3,.Lrender_full_428c
	subu	$8,$8,$16
	neg	$3,$3
.Lrender_full_428c:
	addiu	$11,$3,-256
	bgez	$8,.Lrender_full_429c
	subu	$9,$9,$17
	neg	$8,$8
.Lrender_full_429c:
	addiu	$12,$8,-384
	bgez	$9,.Lrender_full_42ac
	sll	$0,$0,0x0
	neg	$9,$9
.Lrender_full_42ac:
	addiu	$13,$9,-256
	and	$11,$11,$12
	jr	$31
	and	$10,$11,$13
.Lrender_full_42bc:
	lb	$3,19($13)
	lw	$8,0($13)
	lw	$9,4($13)
	lw	$10,8($13)
	sll	$3,$3,0x18
	or	$10,$10,$3
	sw	$8,0($16)
	sw	$9,4($16)
	jr	$31
	sw	$10,8($16)
.if \players == 1
# Ground 4x2 BSP-list bucket entry.
.Lrender_1p_ground_4x2_list:
	lui	$8,0x8009
	addiu	$8,$8,25604
	sw	$8,100($1)
.endif
.Lrender_2p_ground_4x2_list:
	lw	$3,4($4)
.Lrender_full_42f4:
	beq	$4,$0,.Lrender_full_45ec
	sll	$0,$0,0x0
	lw	$24,28($3)
	lw	$25,24($3)
	lw	$10,200($1)
	lhu	$8,60($24)
	sw	$25,104($1)
	srl	$9,$8,0x3
	andi	$9,$9,0x1fc
	addu	$9,$9,$10
	sw	$9,192($1)
	lw	$9,0($9)
	andi	$8,$8,0x1f
	sw	$8,188($1)
	sw	$9,196($1)
.Lrender_full_4330:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,4992
.else
	addiu	$8,$2,8320
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Lrender_full_4364
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Lrender_full_4364:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Lrender_full_45b4
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	cfc2	$3,$31
	swc2	$17,16($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_45d4
	lw	$20,8($24)
	swc2	$13,32($30)
	swc2	$18,36($30)
	swc2	$14,52($30)
	swc2	$19,56($30)
	swc2	$0,0($30)
	swc2	$1,4($30)
	swc2	$2,20($30)
	swc2	$3,24($30)
	swc2	$4,40($30)
	swc2	$5,44($30)
	lwc2	$0,0($22)
	lwc2	$1,4($22)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$2,0($19)
	lwc2	$3,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$4,0($20)
	lwc2	$5,4($20)
	addu	$25,$1,$0
	sll	$0,$0,0x0
	c2	0x280030
	lw	$15,8($22)
	lw	$16,8($19)
	lw	$17,8($20)
	sw	$15,68($30)
	sw	$16,88($30)
	sw	$17,108($30)
	swc2	$12,72($30)
	cfc2	$3,$31
	swc2	$17,76($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_45d4
	lw	$20,12($24)
	swc2	$13,92($30)
	swc2	$18,96($30)
	swc2	$14,112($30)
	swc2	$19,116($30)
	swc2	$0,60($30)
	swc2	$1,64($30)
	swc2	$2,80($30)
	lhu	$21,16($24)
	swc2	$3,84($30)
	swc2	$4,100($30)
	swc2	$5,104($30)
	andi	$19,$20,0xffff
	sll	$19,$19,0x4
	addu	$19,$19,$23
	lwc2	$0,0($19)
	lwc2	$1,4($19)
	srl	$20,$20,0x10
	sll	$20,$20,0x4
	addu	$20,$20,$23
	lwc2	$2,0($20)
	lwc2	$3,4($20)
	lw	$9,20($24)
	sll	$21,$21,0x4
	addu	$21,$21,$23
	lwc2	$4,0($21)
	lwc2	$5,4($21)
	sw	$9,124($1)
	addiu	$8,$0,8
	c2	0x280030
	lw	$15,8($19)
	lw	$16,8($20)
	lw	$17,8($21)
	sw	$15,128($30)
	sw	$16,148($30)
	sw	$17,168($30)
	swc2	$12,132($30)
	cfc2	$3,$31
	swc2	$17,136($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lrender_full_45d4
	swc2	$13,152($30)
	swc2	$18,156($30)
	swc2	$14,172($30)
	swc2	$19,176($30)
	swc2	$0,120($30)
	swc2	$1,124($30)
	swc2	$2,140($30)
	swc2	$3,144($30)
	swc2	$4,160($30)
	swc2	$5,164($30)
	lui	$3,0x50
	ori	$3,$3,0x6478
	jal	.Lrender_full_4c30
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lrender_full_4c30
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lrender_full_4c30
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lrender_full_4c30
	addiu	$10,$0,12
.Lrender_full_45b4:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Lrender_full_4330
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Lrender_full_42f4
	lw	$3,4($4)
.Lrender_full_45d4:
	lw	$8,100($1)
	sll	$0,$0,0x0
	sw	$24,0($8)
	addiu	$8,$8,4
	bgez	$0,.Lrender_full_45b4
	sw	$8,100($1)
.Lrender_full_45ec:
	lw	$8,100($1)
	bgez	$0,.Lrender_full_bucket_continue
	sw	$0,0($8)
.Lrender_full_45f8:
	jr	$31
	sll	$0,$0,0x0
.Lrender_full_4600:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,120
.Lrender_full_4614:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,140
.Lrender_full_4628:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,160
.Lrender_full_463c:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,60
.Lrender_full_4650:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,20
.Lrender_full_4664:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,160
.Lrender_full_4678:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,80
.Lrender_full_468c:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Lrender_full_4b50
	addiu	$22,$30,140
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_46f0
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4600
	addiu	$10,$0,12
	addiu	$3,$1,804
	bne	$3,$30,.Lrender_full_4728
.Lrender_full_46f0:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,80
	addiu	$22,$30,40
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4b50
	addiu	$10,$0,4
.Lrender_full_4728:
	jal	.Lrender_full_4614
	addiu	$10,$0,12
	jal	.Lrender_full_4678
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_468c
	addiu	$10,$0,12
	addiu	$3,$1,804
	bne	$3,$30,.Lrender_full_4788
.Lrender_full_4750:
	addiu	$19,$30,20
	addiu	$20,$30,160
	addiu	$21,$30,0
	addiu	$22,$30,40
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4b50
	addiu	$10,$0,8
.Lrender_full_4788:
	jal	.Lrender_full_4650
	addiu	$10,$0,12
	jal	.Lrender_full_4628
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4664
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_4750
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_463c
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_46f0
	sll	$0,$0,0x0
	jal	.Lrender_full_468c
	addiu	$10,$0,12
	jal	.Lrender_full_4628
	addiu	$10,$0,4
	jal	.Lrender_full_4600
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4614
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_490c
	sll	$0,$0,0x0
	jal	.Lrender_full_4664
	addiu	$10,$0,12
	jal	.Lrender_full_4614
	addiu	$10,$0,4
	jal	.Lrender_full_4600
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4628
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_490c
	sll	$0,$0,0x0
	jal	.Lrender_full_4678
	addiu	$10,$0,12
	jal	.Lrender_full_4628
	addiu	$10,$0,8
	jal	.Lrender_full_4614
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_463c
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_4750
	sll	$0,$0,0x0
	jal	.Lrender_full_4650
	addiu	$10,$0,12
	jal	.Lrender_full_4614
	addiu	$10,$0,8
	jal	.Lrender_full_4628
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_463c
	addiu	$10,$0,12
.Lrender_full_48c0:
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_490c
	sw	$0,180($30)
	jal	.Lrender_full_4600
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Lrender_full_4614
	addiu	$10,$0,12
	addiu	$3,$0,24
	sw	$3,180($30)
	jal	.Lrender_full_4628
	addiu	$10,$0,12
	addiu	$3,$0,36
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_463c
	addiu	$10,$0,12
.Lrender_full_490c:
	sw	$0,180($30)
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_4b50
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_4b50
	addiu	$10,$0,12
.Lrender_full_4950:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	swc2	$14,12($16)
	jr	$31
	swc2	$19,16($16)
.Lrender_full_4a38:
	lh	$11,0($13)
	lh	$12,0($14)
	sh	$11,0($15)
	sh	$12,0($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,0($18)
	lh	$11,2($13)
	lh	$12,2($14)
	sh	$11,2($15)
	sh	$12,2($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sh	$3,2($18)
	lh	$11,4($13)
	lh	$12,4($14)
	sh	$11,4($15)
	sh	$12,4($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	lwc2	$0,0($18)
	mtc2	$3,$1
	sh	$3,4($18)
	lbu	$11,8($13)
	c2	0x180001
	lbu	$12,8($14)
	sb	$11,8($15)
	sb	$12,8($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,8($18)
	lbu	$11,9($13)
	lbu	$12,9($14)
	sb	$11,9($15)
	sb	$12,9($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,9($18)
	lbu	$11,10($13)
	lbu	$12,10($14)
	sb	$11,10($15)
	sb	$12,10($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,10($18)
	lbu	$11,6($13)
	lbu	$12,6($14)
	sb	$11,6($15)
	sb	$12,6($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,6($18)
	lbu	$11,7($13)
	lbu	$12,7($14)
	sb	$11,7($15)
	sb	$12,7($16)
	addu	$3,$11,$12
	sra	$3,$3,0x1
	sb	$3,7($18)
	lw	$3,12($13)
	lw	$8,16($13)
	sw	$3,12($15)
	sw	$8,16($15)
	lw	$3,12($14)
	lw	$8,16($14)
	sw	$3,12($16)
	sw	$8,16($16)
	swc2	$14,12($18)
	jr	$31
	swc2	$19,16($18)
.Lrender_full_4b50:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_4ba4
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_4ba4:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_45f8
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_45f8
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_4be4
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_4be4:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lrender_full_45f8
	sw	$3,112($1)
	sw	$31,60($25)
.if \players == 1
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_4f98
.else
	addiu	$8,$1,804
	beq	$8,$30,.Lrender_full_4f98
.endif
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Lrender_full_1p_4e70
.else
	bgez	$0,.Lrender_full_4e84
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Lrender_full_4c30:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_4ce4
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_4ce4:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_45f8
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_45f8
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_4d24
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_4d24:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_45f8
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Lrender_full_4d78
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_4d78:
	subu	$10,$28,$13
	bgtz	$10,.Lrender_full_4d88
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lrender_full_4d88:
	subu	$10,$28,$14
	bgtz	$10,.Lrender_full_4d98
	lw	$3,28($3)
	addu	$28,$14,$0
.Lrender_full_4d98:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lrender_full_4db4
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lrender_full_4db4:
.if \players == 1
	bgez	$28,.Lrender_full_1p_4dc0
.else
	bgez	$28,.Lrender_full_4dc4
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Lrender_full_1p_4dc0:
	lw	$8,28($1)
.endif
.Lrender_full_4dc4:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Lrender_full_4e04
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_4e04
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_4e04
	sll	$0,$0,0x0
	bltz	$9,.Lrender_full_4e04
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lrender_full_4e04:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lrender_full_4e4c
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_4e6c
.else
	bltz	$10,.Lrender_full_2p_4f34
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Lrender_full_1p_4e6c
.else
	bgez	$0,.Lrender_full_2p_4f34
.endif
	sh	$9,6($22)
.Lrender_full_4e4c:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_4e6c
.else
	bltz	$10,.Lrender_full_2p_4f34
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Lrender_full_1p_4e6c:
	lw	$3,36($1)
.Lrender_full_1p_4e70:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Lrender_full_2p_4f34:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Lrender_full_4e84:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_4f7c
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	addiu	$16,$30,20
	jal	.Lrender_full_4a38
	addiu	$18,$30,80
	addu	$13,$21,$0
	addu	$14,$22,$0
	addiu	$15,$30,40
	addiu	$16,$30,60
	jal	.Lrender_full_4a38
	addiu	$18,$30,160
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_4f4c
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Lrender_full_4950
	addiu	$16,$30,100
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
	jal	.Lrender_full_4950
	addiu	$16,$30,120
	addu	$13,$22,$0
	addu	$14,$20,$0
	addiu	$15,$30,60
	jal	.Lrender_full_4950
	addiu	$16,$30,140
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_4f6c)
	addiu	$8,$8,%lo(.Lrender_full_4f6c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_4f4c:
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_4f6c)
	addiu	$8,$8,%lo(.Lrender_full_4f6c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_4f6c:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lrender_full_4f7c:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lrender_full_4f98:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lrender_full_503c
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lrender_full_5008
	lw	$3,156($1)
	lui	$10,%hi(.Lrender_full_48c0)
	addiu	$10,$10,%lo(.Lrender_full_48c0)
	bne	$3,$10,.Lrender_full_503c
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lrender_full_4fd8
	lw	$10,132($1)
	addiu	$8,$8,96
.Lrender_full_4fd8:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Lrender_full_5008:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Lrender_full_503c:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lrender_full_5008
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_50a8
	lui	$3,0x3600
	lui	$3,0x3400
.Lrender_full_50a8:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Lrender_full_517c
	lui	$10,0x900
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_513c
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lrender_full_513c:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lrender_full_517c:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
# Previously rendered ground 4x2 bucket entry.
.Lrender_1p_ground_4x2_rendered:
.Lrender_2p_ground_4x2_rendered:
	lw	$24,0($4)
.Lrender_full_51a4:
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,4992
.else
	addiu	$8,$2,8320
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	addiu	$30,$1,436
	beq	$24,$0,.Lrender_full_bucket_continue
	DRAW_LEVEL_OVR_PREPARE_RENDERED_QUAD
	jal	.Lrender_full_5a84
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lrender_full_5a84
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lrender_full_5a84
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lrender_full_5a84
	addiu	$10,$0,12
	lw	$24,4($4)
	bgez	$0,.Lrender_full_51a4
	addiu	$4,$4,4
.Lrender_full_54fc:
	jr	$31
	sll	$0,$0,0x0
.Lrender_full_5504:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,120
.Lrender_full_5518:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,140
.Lrender_full_552c:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,160
.Lrender_full_5540:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,60
.Lrender_full_5554:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,20
.Lrender_full_5568:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,160
.Lrender_full_557c:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,80
.Lrender_full_5590:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Lrender_full_5960
	addiu	$22,$30,140
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_55f4
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5504
	addiu	$10,$0,12
	addiu	$3,$1,804
	bne	$3,$30,.Lrender_full_562c
.Lrender_full_55f4:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,80
	addiu	$22,$30,40
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5960
	addiu	$10,$0,4
.Lrender_full_562c:
	jal	.Lrender_full_5518
	addiu	$10,$0,12
	jal	.Lrender_full_557c
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5590
	addiu	$10,$0,12
	addiu	$3,$1,804
	bne	$3,$30,.Lrender_full_568c
.Lrender_full_5654:
	addiu	$19,$30,20
	addiu	$20,$30,160
	addiu	$21,$30,0
	addiu	$22,$30,40
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5960
	addiu	$10,$0,8
.Lrender_full_568c:
	jal	.Lrender_full_5554
	addiu	$10,$0,12
	jal	.Lrender_full_552c
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5568
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_5654
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5540
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_55f4
	sll	$0,$0,0x0
	jal	.Lrender_full_5590
	addiu	$10,$0,12
	jal	.Lrender_full_552c
	addiu	$10,$0,4
	jal	.Lrender_full_5504
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5518
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_5810
	sll	$0,$0,0x0
	jal	.Lrender_full_5568
	addiu	$10,$0,12
	jal	.Lrender_full_5518
	addiu	$10,$0,4
	jal	.Lrender_full_5504
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_552c
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_5810
	sll	$0,$0,0x0
	jal	.Lrender_full_557c
	addiu	$10,$0,12
	jal	.Lrender_full_552c
	addiu	$10,$0,8
	jal	.Lrender_full_5518
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5540
	addiu	$10,$0,12
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_5654
	sll	$0,$0,0x0
	jal	.Lrender_full_5554
	addiu	$10,$0,12
	jal	.Lrender_full_5518
	addiu	$10,$0,8
	jal	.Lrender_full_552c
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5540
	addiu	$10,$0,12
.Lrender_full_57c4:
	addiu	$3,$1,804
	beq	$3,$30,.Lrender_full_5810
	sw	$0,180($30)
	jal	.Lrender_full_5504
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Lrender_full_5518
	addiu	$10,$0,12
	addiu	$3,$0,24
	sw	$3,180($30)
	jal	.Lrender_full_552c
	addiu	$10,$0,12
	addiu	$3,$0,36
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5540
	addiu	$10,$0,12
.Lrender_full_5810:
	sw	$0,180($30)
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lrender_full_5960
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,160
	addiu	$22,$30,60
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lrender_full_5960
	addiu	$10,$0,12
.Lrender_full_5854:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	lw	$11,92($1)
	mfc2	$3,$19
	swc2	$14,12($16)
	sw	$3,16($16)
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	sb	$8,18($16)
	sll	$3,$3,0x1
	subu	$8,$3,$11
	srl	$8,$8,0x1f
	jr	$31
	sb	$8,19($16)
.Lrender_full_5960:
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_54fc
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_5a58
	addiu	$3,$0,12
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_59f8
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_59f8:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_54fc
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_54fc
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_5a38
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_5a38:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lrender_full_54fc
.Lrender_full_5a58:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,804
	beq	$8,$30,.Lrender_full_5e38
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
.if \players == 2
	addiu	$17,$11,-768
	addiu	$18,$12,-768
	addiu	$15,$13,-768
.endif
.if \players == 1
	bgez	$0,.Lrender_full_1p_5d20
.else
	bgez	$0,.Lrender_full_5d34
.endif
.if \players == 1
	lw	$3,40($1)
.else
	addiu	$16,$14,-768
.endif
.Lrender_full_5a84:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_54fc
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_5bd4
	addiu	$3,$0,12
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lrender_full_5b7c
	neg	$8,$8
	xor	$8,$8,$10
.Lrender_full_5b7c:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lrender_full_54fc
	sll	$15,$15,0x10
	bltz	$15,.Lrender_full_54fc
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lrender_full_5bbc
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lrender_full_5bbc:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lrender_full_54fc
.Lrender_full_5bd4:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$25,$25,4
	srl	$9,$9,0x1f
	sll	$9,$9,0x1f
	srl	$10,$10,0x1f
	sll	$10,$10,0x1e
	or	$10,$10,$9
	sw	$10,128($1)
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	lw	$3,180($30)
	subu	$10,$11,$12
	srl	$9,$3,0x2
	addu	$9,$9,$24
	lb	$9,24($9)
	bgtz	$10,.Lrender_full_5c28
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lrender_full_5c28:
	subu	$10,$28,$13
	bgtz	$10,.Lrender_full_5c38
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lrender_full_5c38:
	subu	$10,$28,$14
	bgtz	$10,.Lrender_full_5c48
	lw	$3,28($3)
	addu	$28,$14,$0
.Lrender_full_5c48:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lrender_full_5c64
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lrender_full_5c64:
.if \players == 1
	bgez	$28,.Lrender_full_1p_5c70
.else
	bgez	$28,.Lrender_full_5c74
.endif
	addu	$28,$28,$5
	addu	$28,$5,$0
.if \players == 1
.Lrender_full_1p_5c70:
	lw	$8,28($1)
.endif
.Lrender_full_5c74:
	lw	$9,36($3)
.if \players == 1
	subu	$8,$10,$8
.else
	addiu	$8,$10,-4096
.endif
	sw	$9,132($1)
	bgez	$8,.Lrender_full_5cb4
.if \players == 1
	lw	$8,32($1)
.else
	addiu	$8,$10,-2048
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_5cb4
.if \players == 1
	lw	$8,36($1)
.else
	addiu	$8,$10,-1536
.endif
	addiu	$3,$3,12
.if \players == 1
	subu	$8,$10,$8
.endif
	bgez	$8,.Lrender_full_5cb4
	sll	$0,$0,0x0
	bltz	$9,.Lrender_full_5cb4
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lrender_full_5cb4:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lrender_full_5cfc
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_5d1c
.else
	bltz	$10,.Lrender_full_2p_5ddc
.endif
	srl	$9,$8,0x10
.if \players == 1
	bgez	$0,.Lrender_full_1p_5d1c
.else
	bgez	$0,.Lrender_full_2p_5ddc
.endif
	sh	$9,6($22)
.Lrender_full_5cfc:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
.if \players == 1
	bltz	$10,.Lrender_full_1p_5d1c
.else
	bltz	$10,.Lrender_full_2p_5ddc
.endif
	sh	$8,6($21)
	sh	$9,6($22)
.if \players == 1
.Lrender_full_1p_5d1c:
	lw	$3,36($1)
.Lrender_full_1p_5d20:
	sll	$0,$0,0x0
	subu	$17,$11,$3
	subu	$18,$12,$3
	subu	$15,$13,$3
	subu	$16,$14,$3
.else
.Lrender_full_2p_5ddc:
	addiu	$17,$11,-1536
	addiu	$18,$12,-1536
	addiu	$15,$13,-1536
	addiu	$16,$14,-1536
.endif
.Lrender_full_5d34:
	srl	$17,$17,0x1d
	andi	$17,$17,0x4
	srl	$18,$18,0x1c
	andi	$18,$18,0x8
	or	$17,$17,$18
	srl	$15,$15,0x1b
	andi	$15,$15,0x10
	or	$17,$17,$15
	srl	$16,$16,0x1a
	andi	$16,$16,0x20
	or	$17,$17,$16
	beq	$17,$0,.Lrender_full_5e1c
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Lrender_full_5854
	addiu	$16,$30,80
	addu	$13,$21,$0
	addu	$14,$22,$0
	addiu	$15,$30,40
	jal	.Lrender_full_5854
	addiu	$16,$30,160
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Lrender_full_5854
	addiu	$16,$30,100
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
	jal	.Lrender_full_5854
	addiu	$16,$30,120
	addu	$13,$22,$0
	addu	$14,$20,$0
	addiu	$15,$30,60
	jal	.Lrender_full_5854
	addiu	$16,$30,140
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_5e0c)
	addiu	$8,$8,%lo(.Lrender_full_5e0c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lrender_full_5e0c)
	addiu	$8,$8,%lo(.Lrender_full_5e0c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lrender_full_5e0c:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lrender_full_5e1c:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lrender_full_5e38:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lrender_full_5edc
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lrender_full_5ea8
	lw	$3,156($1)
	lui	$10,%hi(.Lrender_full_57c4)
	addiu	$10,$10,%lo(.Lrender_full_57c4)
	bne	$3,$10,.Lrender_full_5edc
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lrender_full_5e78
	lw	$10,132($1)
	addiu	$8,$8,96
.Lrender_full_5e78:
	addu	$8,$8,$9
	addu	$10,$10,$8
	lw	$8,0($10)
	lw	$9,4($10)
	sw	$8,416($1)
	sh	$8,6($19)
	lh	$8,8($10)
	sw	$9,420($1)
	sh	$9,6($20)
	lh	$9,10($10)
	sh	$8,6($21)
	sh	$9,6($22)
.Lrender_full_5ea8:
	lw	$3,112($1)
	sll	$0,$0,0x0
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	addiu	$25,$25,-4
	lw	$31,60($25)
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	jr	$31
	sw	$9,420($1)
.Lrender_full_5edc:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lrender_full_5ea8
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	and	$3,$15,$16
	and	$3,$3,$17
	bltz	$3,.Lrender_full_54fc
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	or	$15,$15,$16
	or	$15,$15,$17
	bltz	$15,.Lrender_full_60b0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_5f78
	lui	$3,0x3600
	lui	$3,0x3400
.Lrender_full_5f78:
	or	$3,$3,$15
	sw	$3,4($2)
	sw	$11,8($2)
	sw	$19,12($2)
	sw	$16,16($2)
	sw	$12,20($2)
	sw	$20,24($2)
	sw	$17,28($2)
	sw	$13,32($2)
	sw	$21,36($2)
	addiu	$9,$0,40
	bgez	$0,.Lrender_full_608c
	lui	$10,0x900
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lrender_full_54fc
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lrender_full_6154
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lrender_full_604c
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lrender_full_604c:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lrender_full_608c:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.Lrender_full_60b0:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lrender_full_621c
	lh	$9,4($19)
	bltz	$10,.Lrender_full_6100
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lrender_full_621c
	lh	$9,4($20)
	bltz	$10,.Lrender_full_6100
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lrender_full_621c
	lh	$9,4($21)
	lw	$31,88($1)
	bgez	$10,.Lrender_full_54fc
.Lrender_full_6100:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,36
	lw	$31,88($1)
	addiu	$15,$15,48
	jr	$31
	sw	$15,16($1)
.Lrender_full_6154:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lrender_full_621c
	lh	$9,4($19)
	bltz	$10,.Lrender_full_61b8
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lrender_full_621c
	lh	$9,4($20)
	bltz	$10,.Lrender_full_61b8
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lrender_full_621c
	lh	$9,4($21)
	bltz	$10,.Lrender_full_61b8
	lh	$3,0($22)
	lh	$8,2($22)
	jal	.Lrender_full_621c
	lh	$9,4($22)
	lw	$31,88($1)
	bgez	$10,.Lrender_full_54fc
.Lrender_full_61b8:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	ori	$3,$3,0x1
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,36
	addu	$13,$22,$0
	jal	.Lrender_full_625c
	addiu	$16,$15,48
	lw	$31,88($1)
	addiu	$15,$15,60
	jr	$31
	sw	$15,16($1)
.Lrender_full_621c:
	subu	$3,$3,$15
	bgez	$3,.Lrender_full_622c
	subu	$8,$8,$16
	neg	$3,$3
.Lrender_full_622c:
	addiu	$11,$3,-256
	bgez	$8,.Lrender_full_623c
	subu	$9,$9,$17
	neg	$8,$8
.Lrender_full_623c:
	addiu	$12,$8,-384
	bgez	$9,.Lrender_full_624c
	sll	$0,$0,0x0
	neg	$9,$9
.Lrender_full_624c:
	addiu	$13,$9,-256
	and	$11,$11,$12
	jr	$31
	and	$10,$11,$13
.Lrender_full_625c:
	lb	$3,19($13)
	lw	$8,0($13)
	lw	$9,4($13)
	lw	$10,8($13)
	sll	$3,$3,0x18
	or	$10,$10,$3
	sw	$8,0($16)
	sw	$9,4($16)
	jr	$31
	sw	$10,8($16)
# Dynamic BSP-list bucket entry.
	DRAW_LEVEL_OVR_DYNAMIC_LIST \players
# Previously rendered dynamic bucket entry.
	DRAW_LEVEL_OVR_DYNAMIC_RENDERED \players
# Quad 4x4 BSP-list bucket entry.
	DRAW_LEVEL_OVR_QUAD_4X4_LIST \players
# Previously rendered quad 4x4 bucket entry.
	DRAW_LEVEL_OVR_QUAD_4X4_RENDERED \players
.Lrender_full_9ac4:
	lw	$31,672($1)
	sll	$0,$0,0x0
.Lrender_full_9acc:
	jr	$31
	sll	$0,$0,0x0
.Lrender_full_9ad4:
	sw	$31,672($1)
	beq	$23,$24,.Lrender_full_9acc
	lw	$11,40($5)
	lw	$12,44($5)
	lw	$13,48($5)
	lw	$14,52($5)
	lw	$15,56($5)
	lw	$16,60($5)
	lw	$17,64($5)
	lw	$18,68($5)
	ctc2	$11,$8
	ctc2	$12,$9
	ctc2	$13,$10
	ctc2	$14,$11
	ctc2	$15,$12
	ctc2	$16,$13
	ctc2	$17,$14
	ctc2	$18,$15
	lh	$3,32($5)
	lh	$8,34($5)
	lw	$30,24($5)
	sll	$3,$3,0xf
	sll	$8,$8,0xf
	ctc2	$3,$24
	ctc2	$8,$25
	ctc2	$30,$26
	srl	$30,$30,0x1
	addiu	$30,$30,1
	sh	$30,520($1)
	sh	$30,540($1)
	sh	$30,560($1)
	sh	$0,534($1)
	sh	$0,554($1)
	sh	$0,574($1)
	lw	$3,32($5)
	lw	$5,244($5)
	sw	$3,108($1)
	addiu	$11,$0,4096
	ctc2	$11,$0
	ctc2	$0,$1
	ctc2	$11,$2
	ctc2	$0,$3
	ctc2	$11,$4
	ctc2	$0,$5
	ctc2	$0,$6
	ctc2	$0,$7
.Lrender_full_9b8c:
	beq	$23,$24,.Lrender_full_9ac4
	lw	$3,48($1)
.if \players == 1
	addiu	$8,$2,3432
.else
	addiu	$8,$2,6760
.endif
	subu	$3,$3,$8
	bltz	$3,.Lrender_full_terminal_return
	lwc2	$0,12($23)
	lwc2	$1,16($23)
	lh	$8,8($23)
	lh	$9,10($23)
	c2	0x4a2012
	sh	$8,422($1)
	sh	$9,418($1)
	lh	$3,18($23)
	lw	$8,20($23)
	sh	$3,442($1)
	sw	$8,444($1)
	mfc2	$11,$9
	mfc2	$12,$10
	mfc2	$13,$11
	sh	$11,436($1)
	sh	$12,438($1)
	sh	$13,440($1)
	lwc2	$0,24($23)
	lwc2	$1,28($23)
	lh	$3,30($23)
	lw	$8,32($23)
	c2	0x4a2012
	sh	$3,462($1)
	sw	$8,464($1)
	lw	$28,4($23)
	mfc2	$11,$9
	mfc2	$12,$10
	mfc2	$13,$11
	sh	$11,456($1)
	sh	$12,458($1)
	sh	$13,460($1)
	lwc2	$0,36($23)
	lwc2	$1,40($23)
	lw	$9,0($23)
	lh	$3,42($23)
	c2	0x4a2012
	lw	$8,44($23)
	sh	$3,482($1)
	sw	$8,484($1)
	mfc2	$11,$9
	mfc2	$12,$10
	mfc2	$13,$11
	sh	$11,476($1)
	sh	$12,478($1)
	sh	$13,480($1)
	andi	$3,$9,0x1
	bne	$3,$0,.Ldraw_level_ovr_project_fourth_clip_record
	sw	$9,124($1)
	jal	.Ldraw_level_ovr_prepare_clip_record_depth
	addiu	$13,$1,436
	jal	.Ldraw_level_ovr_prepare_clip_record_depth
	addiu	$13,$1,456
	jal	.Ldraw_level_ovr_prepare_clip_record_depth
	addiu	$13,$1,476
	lb	$3,447($1)
	lb	$8,467($1)
	lb	$9,487($1)
	sll	$3,$3,0x2
	sll	$8,$8,0x3
	sll	$9,$9,0x4
	or	$3,$3,$8
	or	$3,$3,$9
	addu	$3,$3,$1
	lw	$3,576($3)
	sb	$0,447($1)
	sb	$0,467($1)
	jr	$3
	sb	$0,487($1)
# GT3/GT4 near-plane clipping dispatch and jump-table setup.
	DRAW_LEVEL_OVR_FULL_CLIP_DISPATCHES \rdata
.end \symbol
.size \symbol, . - \symbol
.endm
