# PSX renderer family shared by the three- and four-player overlays.
# The wrapper supplies the retail symbol, player count, and RDATA symbol.
# NOTE(aalhendi): This is a reconstructed parameterization of retail code, not
# recovered Naughty Dog source.

.include "DrawLevelOvr_common.inc"

# NOTE(aalhendi): Public method macros keep the composition readable;
# their private bodies preserve the literal retail register schedule.

# Private body shared by the split-ground A/B list methods.
.macro DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_BODY name, edge_source
.ifnc \edge_source,projected_edge
.ifnc \edge_source,scratch_grid
	.error "DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_BODY received an invalid edge source"
.endif
.endif
.Lrender_split_\name:
	lw	$3,4($4)
.Ldraw_level_ovr_split_ground_list_\name\()_01:
	beq	$4,$0,.Ldraw_level_ovr_split_ground_list_\name\()_10
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
.Ldraw_level_ovr_split_ground_list_\name\()_02:
	lw	$3,48($1)
	addiu	$8,$2,.Lsplit_prim_limit
	subu	$3,$3,$8
	bltz	$3,.Lsplit_02bc
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Ldraw_level_ovr_split_ground_list_\name\()_03
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Ldraw_level_ovr_split_ground_list_\name\()_03:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Ldraw_level_ovr_split_ground_list_\name\()_08
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	cfc2	$3,$31
	swc2	$17,16($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_09
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
	lw	$9,20($24)
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
	sw	$9,124($1)
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
	bne	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_09
	swc2	$13,92($30)
	swc2	$18,96($30)
	swc2	$14,112($30)
	swc2	$19,116($30)
	swc2	$0,60($30)
	swc2	$1,64($30)
	swc2	$2,80($30)
	swc2	$3,84($30)
	swc2	$4,100($30)
	swc2	$5,104($30)
	lw	$3,64($24)
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,40
	addiu	$22,$30,60
	lw	$8,0($3)
	lw	$9,4($3)
	lw	$10,8($3)
	sw	$0,404($1)
	sw	$8,416($1)
	sw	$9,420($1)
	sw	$10,424($1)
	sh	$8,6($19)
	sh	$9,6($20)
	sh	$10,6($21)
	srl	$10,$10,0x10
	sh	$10,6($22)
	lw	$11,16($30)
	lw	$12,36($30)
	lw	$13,56($30)
	lw	$14,76($30)
	subu	$3,$11,$12
	bgtz	$3,.Ldraw_level_ovr_split_ground_list_\name\()_04
	addu	$28,$11,$0
	addu	$28,$12,$0
.Ldraw_level_ovr_split_ground_list_\name\()_04:
	subu	$3,$28,$13
	bgtz	$3,.Ldraw_level_ovr_split_ground_list_\name\()_05
	lb	$9,124($1)
	addu	$28,$13,$0
.Ldraw_level_ovr_split_ground_list_\name\()_05:
	subu	$3,$28,$14
	bgtz	$3,.Ldraw_level_ovr_split_ground_list_\name\()_06
	srl	$28,$28,0x6
	srl	$28,$14,0x6
.Ldraw_level_ovr_split_ground_list_\name\()_06:
	addu	$28,$28,$9
	sll	$28,$28,0x2
	addu	$28,$28,$5
	addiu	$17,$11,-1920
	addiu	$18,$12,-1920
	addiu	$15,$13,-1920
	addiu	$16,$14,-1920
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
	beq	$17,$0,.Ldraw_level_ovr_split_ground_list_\name\()_07
	addu	$25,$1,$0
	addu	$11,$17,$1
	lw	$11,328($11)
	lw	$20,12($24)
	lhu	$21,16($24)
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
	sll	$21,$21,0x4
	addu	$21,$21,$23
	lwc2	$4,0($21)
	lwc2	$5,4($21)
	lui	$3,%hi(.Ldraw_level_ovr_split_ground_list_\name\()_08)
	addiu	$3,$3,%lo(.Ldraw_level_ovr_split_ground_list_\name\()_08)
	c2	0x280030
	sw	$3,60($25)
	addiu	$25,$25,4
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
	bne	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_09
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
.Ldraw_level_ovr_split_ground_list_\name\()_07:
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
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_08
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_08
	mfc2	$3,$24
	srl	$8,$8,0x1f
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_08
	addu	$3,$3,$1
	lw	$3,388($3)
	sll	$0,$0,0x0
	jalr	$3
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_list_\name\()_08:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Ldraw_level_ovr_split_ground_list_\name\()_02
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_01
	lw	$3,4($4)
.Ldraw_level_ovr_split_ground_list_\name\()_09:
	lw	$8,100($1)
	sll	$0,$0,0x0
	sw	$24,0($8)
	addiu	$8,$8,4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_08
	sw	$8,100($1)
.Ldraw_level_ovr_split_ground_list_\name\()_10:
	lw	$8,100($1)
	bgez	$0,.Lsplit_03f8
	sw	$0,0($8)
.Ldraw_level_ovr_split_ground_list_\name\()_11:
	jr	$31
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_list_\name\()_12:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,120
.Ldraw_level_ovr_split_ground_list_\name\()_13:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,140
.Ldraw_level_ovr_split_ground_list_\name\()_14:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,160
.Ldraw_level_ovr_split_ground_list_\name\()_15:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,60
.Ldraw_level_ovr_split_ground_list_\name\()_16:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,20
.Ldraw_level_ovr_split_ground_list_\name\()_17:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,160
.Ldraw_level_ovr_split_ground_list_\name\()_18:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,80
.Ldraw_level_ovr_split_ground_list_\name\()_19:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_12
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_18
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_19
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_16
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_17
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_23
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_15
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_19
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_12
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_17
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_12
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_18
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_15
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_16
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_15
	addiu	$10,$0,12
.Ldraw_level_ovr_split_ground_list_\name\()_20:
	addiu	$3,$1,436
	bne	$3,$30,.Ldraw_level_ovr_split_ground_list_\name\()_21
	lui	$3,0x50
	ori	$3,$3,0x6478
	addiu	$8,$0,8
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_26
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_26
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_26
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_26
	addiu	$10,$0,12
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_08
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_list_\name\()_21:
	sw	$0,180($30)
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_12
	addiu	$10,$0,12
	addiu	$3,$0,4
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_13
	addiu	$10,$0,12
	addiu	$3,$0,8
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_14
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_15
	addiu	$10,$0,12
.Ldraw_level_ovr_split_ground_list_\name\()_22:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	swc2	$14,12($16)
	jr	$31
	swc2	$19,16($16)
.Ldraw_level_ovr_split_ground_list_\name\()_23:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_split_ground_list_\name\()_24
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_split_ground_list_\name\()_24:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_11
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_11
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_25
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_split_ground_list_\name\()_25:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_11
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,804
	beq	$8,$30,.Ldraw_level_ovr_split_ground_list_\name\()_40
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	addiu	$17,$11,-320
	addiu	$18,$12,-320
	addiu	$15,$13,-320
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_37
	addiu	$16,$14,-320
.Ldraw_level_ovr_split_ground_list_\name\()_26:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_split_ground_list_\name\()_27
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_split_ground_list_\name\()_27:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_11
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_split_ground_list_\name\()_11
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_28
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_split_ground_list_\name\()_28:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Ldraw_level_ovr_split_ground_list_\name\()_11
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
	bgtz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_29
	addu	$28,$11,$0
	addu	$28,$12,$0
.Ldraw_level_ovr_split_ground_list_\name\()_29:
	subu	$10,$28,$13
	bgtz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_30
	addu	$3,$3,$24
	addu	$28,$13,$0
.Ldraw_level_ovr_split_ground_list_\name\()_30:
	subu	$10,$28,$14
	bgtz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_31
	lw	$3,28($3)
	addu	$28,$14,$0
.Ldraw_level_ovr_split_ground_list_\name\()_31:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Ldraw_level_ovr_split_ground_list_\name\()_32
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Ldraw_level_ovr_split_ground_list_\name\()_32:
	bgez	$28,.Ldraw_level_ovr_split_ground_list_\name\()_33
	addu	$28,$28,$5
	addu	$28,$5,$0
.Ldraw_level_ovr_split_ground_list_\name\()_33:
	lw	$9,36($3)
	addiu	$8,$10,-1600
	sw	$9,132($1)
	bgez	$8,.Ldraw_level_ovr_split_ground_list_\name\()_34
	addiu	$8,$10,-1280
	addiu	$3,$3,12
	bgez	$8,.Ldraw_level_ovr_split_ground_list_\name\()_34
	addiu	$8,$10,-640
	addiu	$3,$3,12
	bgez	$8,.Ldraw_level_ovr_split_ground_list_\name\()_34
	sll	$0,$0,0x0
	bltz	$9,.Ldraw_level_ovr_split_ground_list_\name\()_34
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Ldraw_level_ovr_split_ground_list_\name\()_34:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_35
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
	bltz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_36
	srl	$9,$8,0x10
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_36
	sh	$9,6($22)
.Ldraw_level_ovr_split_ground_list_\name\()_35:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
	bltz	$10,.Ldraw_level_ovr_split_ground_list_\name\()_36
	sh	$8,6($21)
	sh	$9,6($22)
.Ldraw_level_ovr_split_ground_list_\name\()_36:
	addiu	$17,$11,-640
	addiu	$18,$12,-640
	addiu	$15,$13,-640
	addiu	$16,$14,-640
.Ldraw_level_ovr_split_ground_list_\name\()_37:
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
	beq	$17,$0,.Ldraw_level_ovr_split_ground_list_\name\()_39
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_22
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_22
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_22
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_22
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
	jal	.Ldraw_level_ovr_split_ground_list_\name\()_22
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Ldraw_level_ovr_split_ground_list_\name\()_38)
	addiu	$8,$8,%lo(.Ldraw_level_ovr_split_ground_list_\name\()_38)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Ldraw_level_ovr_split_ground_list_\name\()_38:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Ldraw_level_ovr_split_ground_list_\name\()_39:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_list_\name\()_40:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Ldraw_level_ovr_split_ground_list_\name\()_43
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Ldraw_level_ovr_split_ground_list_\name\()_42
	lw	$3,156($1)
	lui	$10,%hi(.Ldraw_level_ovr_split_ground_list_\name\()_20)
	addiu	$10,$10,%lo(.Ldraw_level_ovr_split_ground_list_\name\()_20)
	bne	$3,$10,.Ldraw_level_ovr_split_ground_list_\name\()_43
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Ldraw_level_ovr_split_ground_list_\name\()_41
	lw	$10,132($1)
	addiu	$8,$8,192
.Ldraw_level_ovr_split_ground_list_\name\()_41:
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
.Ldraw_level_ovr_split_ground_list_\name\()_42:
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
.Ldraw_level_ovr_split_ground_list_\name\()_43:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_42
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_split_ground_list_\name\()_44
	lui	$3,0x3600
	lui	$3,0x3400
.Ldraw_level_ovr_split_ground_list_\name\()_44:
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
	bgez	$0,.Ldraw_level_ovr_split_ground_list_\name\()_46
	lui	$10,0x900
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_split_ground_list_\name\()_45
	lui	$3,0x3e00
	lui	$3,0x3c00
.Ldraw_level_ovr_split_ground_list_\name\()_45:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Ldraw_level_ovr_split_ground_list_\name\()_46:
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

.macro DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_A
	DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_BODY ground_list_a, projected_edge
.endm

.macro DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_B
	DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_BODY ground_list_b, scratch_grid
.endm

# Private body shared by the split-ground A/B rendered methods.
.macro DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_BODY name, edge_source
.ifnc \edge_source,projected_edge
.ifnc \edge_source,scratch_grid
	.error "DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_BODY received an invalid edge source"
.endif
.endif
.Lrender_split_\name:
	lw	$24,0($4)
.Ldraw_level_ovr_split_ground_rendered_\name\()_01:
	lw	$3,48($1)
	addiu	$8,$2,.Lsplit_prim_limit
	subu	$3,$3,$8
	bltz	$3,.Lsplit_02bc
	addiu	$30,$1,436
	beq	$24,$0,.Lsplit_03f8
	DRAW_LEVEL_OVR_PREPARE_RENDERED_QUAD
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_17
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_17
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_17
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_17
	addiu	$10,$0,12
	lw	$24,4($4)
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_01
	addiu	$4,$4,4
.Ldraw_level_ovr_split_ground_rendered_\name\()_02:
	jr	$31
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_rendered_\name\()_03:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,120
.Ldraw_level_ovr_split_ground_rendered_\name\()_04:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,140
.Ldraw_level_ovr_split_ground_rendered_\name\()_05:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,160
.Ldraw_level_ovr_split_ground_rendered_\name\()_06:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,60
.Ldraw_level_ovr_split_ground_rendered_\name\()_07:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,20
.Ldraw_level_ovr_split_ground_rendered_\name\()_08:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,160
.Ldraw_level_ovr_split_ground_rendered_\name\()_09:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,80
.Ldraw_level_ovr_split_ground_rendered_\name\()_10:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_03
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_09
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_10
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_07
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_08
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_13
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_06
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_10
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_03
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_08
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,4
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_03
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_09
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_06
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_07
	addiu	$10,$0,12
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,8
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_06
	addiu	$10,$0,12
.Ldraw_level_ovr_split_ground_rendered_\name\()_11:
	sw	$0,180($30)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_03
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_04
	addiu	$10,$0,12
	addiu	$3,$0,48
	sw	$3,180($30)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_05
	addiu	$10,$0,12
	addiu	$3,$0,60
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_06
	addiu	$10,$0,12
.Ldraw_level_ovr_split_ground_rendered_\name\()_12:
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
.Ldraw_level_ovr_split_ground_rendered_\name\()_13:
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_16
	addiu	$3,$0,12
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_14
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_split_ground_rendered_\name\()_14:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_15
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_split_ground_rendered_\name\()_15:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
.Ldraw_level_ovr_split_ground_rendered_\name\()_16:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,804
	beq	$8,$30,.Ldraw_level_ovr_split_ground_rendered_\name\()_32
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	addiu	$17,$11,-320
	addiu	$18,$12,-320
	addiu	$15,$13,-320
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_29
	addiu	$16,$14,-320
.Ldraw_level_ovr_split_ground_rendered_\name\()_17:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_20
	addiu	$3,$0,12
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_18
	neg	$8,$8
	xor	$8,$8,$10
.Ldraw_level_ovr_split_ground_rendered_\name\()_18:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	sll	$15,$15,0x10
	bltz	$15,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_19
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Ldraw_level_ovr_split_ground_rendered_\name\()_19:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
.Ldraw_level_ovr_split_ground_rendered_\name\()_20:
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
	bgtz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_21
	addu	$28,$11,$0
	addu	$28,$12,$0
.Ldraw_level_ovr_split_ground_rendered_\name\()_21:
	subu	$10,$28,$13
	bgtz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_22
	addu	$3,$3,$24
	addu	$28,$13,$0
.Ldraw_level_ovr_split_ground_rendered_\name\()_22:
	subu	$10,$28,$14
	bgtz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_23
	lw	$3,28($3)
	addu	$28,$14,$0
.Ldraw_level_ovr_split_ground_rendered_\name\()_23:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_24
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Ldraw_level_ovr_split_ground_rendered_\name\()_24:
	bgez	$28,.Ldraw_level_ovr_split_ground_rendered_\name\()_25
	addu	$28,$28,$5
	addu	$28,$5,$0
.Ldraw_level_ovr_split_ground_rendered_\name\()_25:
	lw	$9,36($3)
	addiu	$8,$10,-1600
	sw	$9,132($1)
	bgez	$8,.Ldraw_level_ovr_split_ground_rendered_\name\()_26
	addiu	$8,$10,-1280
	addiu	$3,$3,12
	bgez	$8,.Ldraw_level_ovr_split_ground_rendered_\name\()_26
	addiu	$8,$10,-640
	addiu	$3,$3,12
	bgez	$8,.Ldraw_level_ovr_split_ground_rendered_\name\()_26
	sll	$0,$0,0x0
	bltz	$9,.Ldraw_level_ovr_split_ground_rendered_\name\()_26
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Ldraw_level_ovr_split_ground_rendered_\name\()_26:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_27
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_28
	srl	$9,$8,0x10
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_28
	sh	$9,6($22)
.Ldraw_level_ovr_split_ground_rendered_\name\()_27:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_28
	sh	$8,6($21)
	sh	$9,6($22)
.Ldraw_level_ovr_split_ground_rendered_\name\()_28:
	addiu	$17,$11,-640
	addiu	$18,$12,-640
	addiu	$15,$13,-640
	addiu	$16,$14,-640
.Ldraw_level_ovr_split_ground_rendered_\name\()_29:
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
	beq	$17,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_31
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_12
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_12
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_12
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_12
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
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_12
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Ldraw_level_ovr_split_ground_rendered_\name\()_30)
	addiu	$8,$8,%lo(.Ldraw_level_ovr_split_ground_rendered_\name\()_30)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Ldraw_level_ovr_split_ground_rendered_\name\()_30:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Ldraw_level_ovr_split_ground_rendered_\name\()_31:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Ldraw_level_ovr_split_ground_rendered_\name\()_32:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_35
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Ldraw_level_ovr_split_ground_rendered_\name\()_34
	lw	$3,156($1)
	lui	$10,%hi(.Ldraw_level_ovr_split_ground_rendered_\name\()_11)
	addiu	$10,$10,%lo(.Ldraw_level_ovr_split_ground_rendered_\name\()_11)
	bne	$3,$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_35
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_33
	lw	$10,132($1)
	addiu	$8,$8,192
.Ldraw_level_ovr_split_ground_rendered_\name\()_33:
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
.Ldraw_level_ovr_split_ground_rendered_\name\()_34:
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
.Ldraw_level_ovr_split_ground_rendered_\name\()_35:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_34
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	and	$3,$15,$16
	and	$3,$3,$17
	bltz	$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	or	$15,$15,$16
	or	$15,$15,$17
	bltz	$15,.Ldraw_level_ovr_split_ground_rendered_\name\()_39
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_36
	lui	$3,0x3600
	lui	$3,0x3400
.Ldraw_level_ovr_split_ground_rendered_\name\()_36:
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
	bgez	$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_38
	lui	$10,0x900
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Ldraw_level_ovr_split_ground_rendered_\name\()_41
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_37
	lui	$3,0x3e00
	lui	$3,0x3c00
.Ldraw_level_ovr_split_ground_rendered_\name\()_37:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Ldraw_level_ovr_split_ground_rendered_\name\()_38:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.Ldraw_level_ovr_split_ground_rendered_\name\()_39:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($19)
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_40
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($20)
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_40
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($21)
	lw	$31,88($1)
	bgez	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
.Ldraw_level_ovr_split_ground_rendered_\name\()_40:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,36
	lw	$31,88($1)
	addiu	$15,$15,48
	jr	$31
	sw	$15,16($1)
.Ldraw_level_ovr_split_ground_rendered_\name\()_41:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($19)
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_42
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($20)
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_42
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($21)
	bltz	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_42
	lh	$3,0($22)
	lh	$8,2($22)
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_43
	lh	$9,4($22)
	lw	$31,88($1)
	bgez	$10,.Ldraw_level_ovr_split_ground_rendered_\name\()_02
.Ldraw_level_ovr_split_ground_rendered_\name\()_42:
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
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,36
	addu	$13,$22,$0
	jal	.Ldraw_level_ovr_split_ground_rendered_\name\()_47
	addiu	$16,$15,48
	lw	$31,88($1)
	addiu	$15,$15,60
	jr	$31
	sw	$15,16($1)
.Ldraw_level_ovr_split_ground_rendered_\name\()_43:
	subu	$3,$3,$15
	bgez	$3,.Ldraw_level_ovr_split_ground_rendered_\name\()_44
	subu	$8,$8,$16
	neg	$3,$3
.Ldraw_level_ovr_split_ground_rendered_\name\()_44:
	addiu	$11,$3,-256
	bgez	$8,.Ldraw_level_ovr_split_ground_rendered_\name\()_45
	subu	$9,$9,$17
	neg	$8,$8
.Ldraw_level_ovr_split_ground_rendered_\name\()_45:
	addiu	$12,$8,-384
	bgez	$9,.Ldraw_level_ovr_split_ground_rendered_\name\()_46
	sll	$0,$0,0x0
	neg	$9,$9
.Ldraw_level_ovr_split_ground_rendered_\name\()_46:
	addiu	$13,$9,-256
	and	$11,$11,$12
	jr	$31
	and	$10,$11,$13
.Ldraw_level_ovr_split_ground_rendered_\name\()_47:
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

.macro DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_A
	DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_BODY ground_rendered_a, projected_edge
.endm

.macro DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_B
	DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_BODY ground_rendered_b, scratch_grid
.endm

.macro DRAW_LEVEL_OVR_SPLIT_CLIP_DISPATCHES rdata
	DRAW_LEVEL_OVR_CLIP_DISPATCH_BODY split, \rdata, 960, .Lsplit_75b8
.endm

.macro DRAW_LEVEL_OVR_SPLIT_WATER_BSP_LIST
	DRAW_LEVEL_OVR_WATER_BSP_LIST_BODY split, 0, .Lsplit_02bc, .Lsplit_03f8, .Lsplit_prim_limit
.endm

.macro DRAW_LEVEL_OVR_SPLIT_WATER_RENDERED
	DRAW_LEVEL_OVR_WATER_RENDERED_BODY split, 0, .Lsplit_02bc, .Lsplit_03f8, .Lsplit_prim_limit
.endm

.macro DRAW_LEVEL_OVR_SPLIT symbol, players, rdata
.if \players != 3
.if \players != 4
	.error "DRAW_LEVEL_OVR_SPLIT requires three or four players"
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
.if \players == 3
	.set .Lsplit_prim_limit,10816
	.set .Lsplit_final_prim_limit,10088
	.set .Lsplit_last_viewport_state,26116
	.set .Lsplit_after_viewport_2_empty,.Lsplit_0254
	.set .Lsplit_after_viewport_2_done,.Lsplit_0260
.else
	.set .Lsplit_prim_limit,14144
	.set .Lsplit_final_prim_limit,13416
	.set .Lsplit_last_viewport_state,26372
	.set .Lsplit_after_viewport_2_empty,.Lsplit_4p_0270
	.set .Lsplit_after_viewport_2_done,.Lsplit_4p_029c
.endif
	DRAW_LEVEL_OVR_SAVE_CONTEXT
	lw	$3,76($29)
	lw	$8,80($29)
	lw	$9,84($29)
	lw	$10,88($29)
	beq	$3,$0,.Lsplit_02bc
	sw	$3,204($1)
	beq	$8,$0,.Lsplit_02bc
	sw	$8,208($1)
	beq	$9,$0,.Lsplit_02bc
	sw	$9,212($1)
.if \players == 4
	beq	$10,$0,.Lsplit_02bc
.endif
.if \players == 3
	sw	$10,136($1)
.else
	sw	$10,216($1)
.endif
.if \players == 4
	lw	$10,92($29)
.endif
	lw	$2,12($7)
.if \players == 4
	sw	$10,136($1)
.endif
	lw	$3,12($6)
	lw	$8,8($7)
	lw	$29,20($7)
	beq	$3,$0,.Lsplit_02bc
	sw	$8,48($1)
	sw	$5,220($1)
	addiu	$5,$5,272
	sw	$5,224($1)
	addiu	$5,$5,272
	sw	$5,228($1)
.if \players == 4
	addiu	$5,$5,272
	sw	$5,232($1)
.endif
	lui	$10,0x8008
	addiu	$10,$10,16936
	lw	$3,0($10)
	lw	$8,4($10)
	lw	$9,8($10)
.if \players == 4
	lw	$10,12($10)
.endif
	sw	$3,0($1)
	sw	$8,4($1)
	sw	$9,8($1)
.if \players == 4
	sw	$10,12($1)
.endif
	addiu	$13,$0,548
	addiu	$11,$1,436
.Lsplit_00b8:
	sw	$0,0($11)
	addiu	$11,$11,4
	bgtz	$13,.Lsplit_00b8
	addiu	$13,$13,-4
	addiu	$13,$0,23
	lui	$11,%hi(\rdata + 64)
	addiu	$11,$11,%lo(\rdata + 64)
	addiu	$12,$1,236
.Lsplit_00d8:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lsplit_00d8
	addiu	$13,$13,-1
	lw	$23,16($6)
	sw	$4,96($1)
	addiu	$3,$0,28
.Lsplit_00fc:
	bltz	$3,.Lsplit_0270
	addu	$8,$3,$4
	lw	$4,0($8)
	sw	$3,52($1)
	beq	$4,$0,.Lsplit_0154
	sll	$0,$0,0x0
	jal	.Lsplit_02fc
	sll	$0,$0,0x0
	lw	$10,204($1)
	lw	$9,0($1)
	lui	$8,0x8009
	addiu	$8,$8,25604
	jal	.Lsplit_0358
	lw	$5,220($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lsplit_0148)
	addiu	$9,$9,%lo(.Lsplit_0148)
	jr	$8
	sw	$9,84($1)
.Lsplit_0148:
	lw	$9,16($1)
	bgez	$0,.Lsplit_0180
	sw	$9,0($1)
.Lsplit_0154:
	lw	$4,96($1)
	lw	$3,52($1)
	lui	$9,0x8009
	addiu	$9,$9,25604
	addu	$8,$3,$4
	lw	$4,48($8)
	sw	$0,0($9)
	beq	$4,$0,.Lsplit_01d4
	lui	$31,%hi(.Lsplit_019c)
	bgez	$0,.Lsplit_02fc
	ori	$31,$31,%lo(.Lsplit_019c)
.Lsplit_0180:
	lw	$4,96($1)
	lw	$3,52($1)
	sll	$0,$0,0x0
	addu	$8,$3,$4
	lw	$4,48($8)
	sll	$0,$0,0x0
	beq	$4,$0,.Lsplit_01d4
.Lsplit_019c:
	lw	$10,208($1)
	lw	$9,4($1)
	lui	$8,0x8009
	addiu	$8,$8,25860
	jal	.Lsplit_0358
	lw	$5,224($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lsplit_01c8)
	addiu	$9,$9,%lo(.Lsplit_01c8)
	jr	$8
	sw	$9,84($1)
.Lsplit_01c8:
	lw	$9,16($1)
	bgez	$0,.Lsplit_0200
	sw	$9,4($1)
.Lsplit_01d4:
	lw	$4,96($1)
	lw	$3,52($1)
	lui	$9,0x8009
	addiu	$9,$9,25860
	addu	$8,$3,$4
	lw	$4,96($8)
	sw	$0,0($9)
	beq	$4,$0,.Lsplit_after_viewport_2_empty
	lui	$31,%hi(.Lsplit_021c)
	bgez	$0,.Lsplit_02fc
	ori	$31,$31,%lo(.Lsplit_021c)
.Lsplit_0200:
	lw	$4,96($1)
	lw	$3,52($1)
	sll	$0,$0,0x0
	addu	$8,$3,$4
	lw	$4,96($8)
	sll	$0,$0,0x0
	beq	$4,$0,.Lsplit_after_viewport_2_empty
.Lsplit_021c:
	lw	$10,212($1)
	lw	$9,8($1)
	lui	$8,0x8009
	addiu	$8,$8,26116
	jal	.Lsplit_0358
	lw	$5,228($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lsplit_0248)
	addiu	$9,$9,%lo(.Lsplit_0248)
	jr	$8
	sw	$9,84($1)
.Lsplit_0248:
	lw	$9,16($1)
	bgez	$0,.Lsplit_after_viewport_2_done
	sw	$9,8($1)
.if \players == 4
.Lsplit_4p_0270:
	lw	$4,96($1)
	lw	$3,52($1)
	lui	$9,0x8009
	addiu	$9,$9,26116
	addu	$8,$3,$4
	lw	$4,144($8)
	sw	$0,0($9)
	beq	$4,$0,.Lsplit_0254
	lui	$31,%hi(.Lsplit_4p_02b8)
	bgez	$0,.Lsplit_02fc
	ori	$31,$31,%lo(.Lsplit_4p_02b8)
.Lsplit_4p_029c:
	lw	$4,96($1)
	lw	$3,52($1)
	sll	$0,$0,0x0
	addu	$8,$3,$4
	lw	$4,144($8)
	sll	$0,$0,0x0
	beq	$4,$0,.Lsplit_0254
.Lsplit_4p_02b8:
	lw	$10,216($1)
	lw	$9,12($1)
	lui	$8,0x8009
	addiu	$8,$8,26372
	jal	.Lsplit_0358
	lw	$5,232($1)
	lw	$8,116($1)
	lui	$9,%hi(.Lsplit_4p_02e4)
	addiu	$9,$9,%lo(.Lsplit_4p_02e4)
	jr	$8
	sw	$9,84($1)
.Lsplit_4p_02e4:
	lw	$9,16($1)
	bgez	$0,.Lsplit_0260
	sw	$9,12($1)
.endif
.Lsplit_0254:
	lui	$9,0x8009
	addiu	$9,$9,.Lsplit_last_viewport_state
	sw	$0,0($9)
.Lsplit_0260:
	lw	$4,96($1)
	lw	$3,52($1)
	bgez	$0,.Lsplit_00fc
	addiu	$3,$3,-4
.Lsplit_0270:
	jal	.Ldraw_level_ovr_copy_clip_record_jump_table
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 0, .Lsplit_7500, 0
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 1, .Lsplit_7500, 0
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 2, .Lsplit_7500, 0
.if \players == 4
	DRAW_LEVEL_OVR_RENDER_VIEWPORT 3, .Lsplit_7500, 0
.endif
.Lsplit_02bc:
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
.Lsplit_02fc:
	lui	$8,%hi(\rdata + 0)
	addiu	$8,$8,%lo(\rdata + 0)
	addu	$8,$8,$3
	lw	$10,0($8)
	lui	$8,%hi(\rdata + 32)
	addiu	$8,$8,%lo(\rdata + 32)
	addu	$8,$8,$3
	lw	$8,0($8)
.Lsplit_031c:
	lw	$13,0($10)
	lw	$11,4($10)
	lw	$12,8($10)
	beq	$13,$0,.Lsplit_0350
	addu	$12,$12,$1
.Lsplit_0330:
	lw	$14,0($11)
	addiu	$11,$11,4
	sw	$14,0($12)
	addiu	$12,$12,4
	bne	$13,$0,.Lsplit_0330
	addiu	$13,$13,-1
	bgez	$0,.Lsplit_031c
	addi	$10,$10,12
.Lsplit_0350:
	jr	$31
	sw	$8,116($1)
.Lsplit_0358:
	sw	$8,100($1)
	sw	$9,16($1)
	sw	$10,200($1)
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
	jr	$31
	sw	$3,108($1)
.Lsplit_03f8:
	lw	$31,84($1)
	sll	$0,$0,0x0
	jr	$31
	sll	$0,$0,0x0
# Water BSP-list bucket entry.
	DRAW_LEVEL_OVR_SPLIT_WATER_BSP_LIST
# Previously rendered water bucket entry.
	DRAW_LEVEL_OVR_SPLIT_WATER_RENDERED
# Split-ground A BSP-list bucket entry.
	DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_A
# Previously rendered split-ground A bucket entry.
	DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_A
# Split-ground B BSP-list bucket entry.
	DRAW_LEVEL_OVR_SPLIT_GROUND_LIST_B
# Previously rendered split-ground B bucket entry.
	DRAW_LEVEL_OVR_SPLIT_GROUND_RENDERED_B
# Wide-dynamic BSP-list bucket entry.
.Lrender_split_wide_dynamic:
	lw	$3,4($4)
.Lsplit_58f0:
	beq	$4,$0,.Lsplit_5be8
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
.Lsplit_592c:
	lw	$3,48($1)
	addiu	$8,$2,.Lsplit_prim_limit
	subu	$3,$3,$8
	bltz	$3,.Lsplit_02bc
	lw	$3,188($1)
	lw	$8,196($1)
	bgez	$3,.Lsplit_5960
	lw	$9,192($1)
	addiu	$3,$0,31
	lw	$8,4($9)
	addiu	$9,$9,4
	sw	$9,192($1)
	sw	$8,196($1)
.Lsplit_5960:
	sllv	$8,$8,$3
	addiu	$3,$3,-1
	bgez	$8,.Lsplit_5bb0
	sw	$3,188($1)
	addiu	$30,$1,436
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_VERTICES
	swc2	$12,12($30)
	cfc2	$3,$31
	swc2	$17,16($30)
	sll	$3,$3,0xe
	srl	$3,$3,0x1f
	bne	$3,$0,.Lsplit_5bd0
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
	bne	$3,$0,.Lsplit_5bd0
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
	bne	$3,$0,.Lsplit_5bd0
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
	jal	.Lsplit_6014
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lsplit_6014
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lsplit_6014
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lsplit_6014
	addiu	$10,$0,12
.Lsplit_5bb0:
	lw	$25,104($1)
	addiu	$24,$24,92
	addiu	$25,$25,-1
	bgtz	$25,.Lsplit_592c
	sw	$25,104($1)
	lw	$4,0($4)
	bgez	$0,.Lsplit_58f0
	lw	$3,4($4)
.Lsplit_5bd0:
	lw	$8,100($1)
	sll	$0,$0,0x0
	sw	$24,0($8)
	addiu	$8,$8,4
	bgez	$0,.Lsplit_5bb0
	sw	$8,100($1)
.Lsplit_5be8:
	lw	$8,100($1)
	bgez	$0,.Lsplit_03f8
	sw	$0,0($8)
.Lsplit_5bf4:
	jr	$31
	sll	$0,$0,0x0
.Lsplit_5bfc:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,120
.Lsplit_5c10:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,140
.Lsplit_5c24:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,160
.Lsplit_5c38:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,60
.Lsplit_5c4c:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,20
.Lsplit_5c60:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,160
.Lsplit_5c74:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,80
.Lsplit_5c88:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Lsplit_5f28
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Lsplit_5f28
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lsplit_5f28
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5bfc
	addiu	$10,$0,12
	jal	.Lsplit_5c10
	addiu	$10,$0,12
	jal	.Lsplit_5c74
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c88
	addiu	$10,$0,12
	jal	.Lsplit_5c4c
	addiu	$10,$0,12
	jal	.Lsplit_5c24
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c60
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Lsplit_5f28
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lsplit_5f28
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c38
	addiu	$10,$0,12
	jal	.Lsplit_5c88
	addiu	$10,$0,12
	jal	.Lsplit_5c24
	addiu	$10,$0,4
	jal	.Lsplit_5bfc
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c10
	addiu	$10,$0,12
	jal	.Lsplit_5c60
	addiu	$10,$0,12
	jal	.Lsplit_5c10
	addiu	$10,$0,4
	jal	.Lsplit_5bfc
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c24
	addiu	$10,$0,12
	jal	.Lsplit_5c74
	addiu	$10,$0,12
	jal	.Lsplit_5c24
	addiu	$10,$0,8
	jal	.Lsplit_5c10
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c38
	addiu	$10,$0,12
	jal	.Lsplit_5c4c
	addiu	$10,$0,12
	jal	.Lsplit_5c10
	addiu	$10,$0,8
	jal	.Lsplit_5c24
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c38
	addiu	$10,$0,12
.Lsplit_5dfc:
	sw	$0,180($30)
	jal	.Lsplit_5bfc
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Lsplit_5c10
	addiu	$10,$0,12
	addiu	$3,$0,48
	sw	$3,180($30)
	jal	.Lsplit_5c24
	addiu	$10,$0,12
	addiu	$3,$0,60
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_5c38
	addiu	$10,$0,12
.Lsplit_5e40:
	DRAW_LEVEL_OVR_INTERPOLATE_EDGE_VERTEX
	swc2	$14,12($16)
	jr	$31
	swc2	$19,16($16)
.Lsplit_5f28:
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lsplit_5f7c
	neg	$8,$8
	xor	$8,$8,$10
.Lsplit_5f7c:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lsplit_5bf4
	sll	$15,$15,0x10
	bltz	$15,.Lsplit_5bf4
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lsplit_5fbc
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lsplit_5fbc:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lsplit_5bf4
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,804
	beq	$8,$30,.Lsplit_6338
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	addiu	$17,$11,-320
	addiu	$18,$12,-320
	addiu	$15,$13,-320
	bgez	$0,.Lsplit_6254
	addiu	$16,$14,-320
.Lsplit_6014:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lsplit_60c8
	neg	$8,$8
	xor	$8,$8,$10
.Lsplit_60c8:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lsplit_5bf4
	sll	$15,$15,0x10
	bltz	$15,.Lsplit_5bf4
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lsplit_6108
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lsplit_6108:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lsplit_5bf4
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
	bgtz	$10,.Lsplit_615c
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lsplit_615c:
	subu	$10,$28,$13
	bgtz	$10,.Lsplit_616c
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lsplit_616c:
	subu	$10,$28,$14
	bgtz	$10,.Lsplit_617c
	lw	$3,28($3)
	addu	$28,$14,$0
.Lsplit_617c:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lsplit_6198
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lsplit_6198:
	bgez	$28,.Lsplit_61a4
	addu	$28,$28,$5
	addu	$28,$5,$0
.Lsplit_61a4:
	lw	$9,36($3)
	addiu	$8,$10,-1600
	sw	$9,132($1)
	bgez	$8,.Lsplit_61dc
	addiu	$8,$10,-1280
	addiu	$3,$3,12
	bgez	$8,.Lsplit_61dc
	addiu	$8,$10,-640
	addiu	$3,$3,12
	bgez	$8,.Lsplit_61dc
	sll	$0,$0,0x0
	bltz	$9,.Lsplit_61dc
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lsplit_61dc:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lsplit_6224
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
	bltz	$10,.Lsplit_6244
	srl	$9,$8,0x10
	bgez	$0,.Lsplit_6244
	sh	$9,6($22)
.Lsplit_6224:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
	bltz	$10,.Lsplit_6244
	sh	$8,6($21)
	sh	$9,6($22)
.Lsplit_6244:
	addiu	$17,$11,-640
	addiu	$18,$12,-640
	addiu	$15,$13,-640
	addiu	$16,$14,-640
.Lsplit_6254:
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
	beq	$17,$0,.Lsplit_631c
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Lsplit_5e40
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Lsplit_5e40
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Lsplit_5e40
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Lsplit_5e40
	addiu	$16,$30,160
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
	jal	.Lsplit_5e40
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lsplit_630c)
	addiu	$8,$8,%lo(.Lsplit_630c)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lsplit_630c:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lsplit_631c:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lsplit_6338:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lsplit_63dc
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lsplit_63a8
	lw	$3,156($1)
	lui	$10,%hi(.Lsplit_5dfc)
	addiu	$10,$10,%lo(.Lsplit_5dfc)
	bne	$3,$10,.Lsplit_63dc
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lsplit_6378
	lw	$10,132($1)
	addiu	$8,$8,192
.Lsplit_6378:
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
.Lsplit_63a8:
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
.Lsplit_63dc:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lsplit_63a8
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lsplit_6448
	lui	$3,0x3600
	lui	$3,0x3400
.Lsplit_6448:
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
	bgez	$0,.Lsplit_651c
	lui	$10,0x900
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lsplit_64dc
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lsplit_64dc:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lsplit_651c:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
# Previously rendered quad 4x4 bucket entry.
.Lrender_split_quad_4x4_rendered:
	lw	$24,0($4)
.Lsplit_6544:
	lw	$3,48($1)
	addiu	$8,$2,.Lsplit_prim_limit
	subu	$3,$3,$8
	bltz	$3,.Lsplit_02bc
	addiu	$30,$1,436
	beq	$24,$0,.Lsplit_03f8
	DRAW_LEVEL_OVR_PREPARE_RENDERED_QUAD
	jal	.Lsplit_6d24
	addiu	$10,$0,0
	lui	$3,0x5014
	ori	$3,$3,0x788c
	addiu	$8,$0,13
	jal	.Lsplit_6d24
	addiu	$10,$0,4
	lui	$3,0x6478
	ori	$3,$3,0x28a0
	addiu	$8,$0,18
	jal	.Lsplit_6d24
	addiu	$10,$0,8
	lui	$3,0x788c
	ori	$3,$3,0xa03c
	addiu	$8,$0,23
	jal	.Lsplit_6d24
	addiu	$10,$0,12
	lw	$24,4($4)
	bgez	$0,.Lsplit_6544
	addiu	$4,$4,4
.Lsplit_689c:
	jr	$31
	sll	$0,$0,0x0
.Lsplit_68a4:
	addiu	$19,$30,0
	addiu	$20,$30,80
	addiu	$21,$30,100
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,120
.Lsplit_68b8:
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,140
.Lsplit_68cc:
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,160
.Lsplit_68e0:
	addiu	$19,$30,120
	addiu	$20,$30,140
	addiu	$21,$30,160
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,60
.Lsplit_68f4:
	addiu	$19,$30,100
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,20
.Lsplit_6908:
	addiu	$19,$30,20
	addiu	$20,$30,60
	addiu	$21,$30,120
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,160
.Lsplit_691c:
	addiu	$19,$30,40
	addiu	$20,$30,0
	addiu	$21,$30,120
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,80
.Lsplit_6930:
	addiu	$19,$30,40
	addiu	$20,$30,120
	addiu	$21,$30,60
	bgez	$0,.Lsplit_6bf4
	addiu	$22,$30,140
	addiu	$19,$30,80
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,60
	jal	.Lsplit_6bf4
	addiu	$10,$0,12
	addiu	$19,$30,100
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,60
	jal	.Lsplit_6bf4
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68a4
	addiu	$10,$0,12
	jal	.Lsplit_68b8
	addiu	$10,$0,12
	jal	.Lsplit_691c
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_6930
	addiu	$10,$0,12
	jal	.Lsplit_68f4
	addiu	$10,$0,12
	jal	.Lsplit_68cc
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_6908
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,20
	addiu	$21,$30,120
	addiu	$22,$30,140
	jal	.Lsplit_6bf4
	addiu	$10,$0,12
	addiu	$19,$30,0
	addiu	$20,$30,120
	addiu	$21,$30,40
	addiu	$22,$30,160
	jal	.Lsplit_6bf4
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68e0
	addiu	$10,$0,12
	jal	.Lsplit_6930
	addiu	$10,$0,12
	jal	.Lsplit_68cc
	addiu	$10,$0,4
	jal	.Lsplit_68a4
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68b8
	addiu	$10,$0,12
	jal	.Lsplit_6908
	addiu	$10,$0,12
	jal	.Lsplit_68b8
	addiu	$10,$0,4
	jal	.Lsplit_68a4
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68cc
	addiu	$10,$0,12
	jal	.Lsplit_691c
	addiu	$10,$0,12
	jal	.Lsplit_68cc
	addiu	$10,$0,8
	jal	.Lsplit_68b8
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68e0
	addiu	$10,$0,12
	jal	.Lsplit_68f4
	addiu	$10,$0,12
	jal	.Lsplit_68b8
	addiu	$10,$0,8
	jal	.Lsplit_68cc
	addiu	$10,$0,12
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68e0
	addiu	$10,$0,12
.Lsplit_6aa4:
	sw	$0,180($30)
	jal	.Lsplit_68a4
	addiu	$10,$0,12
	addiu	$3,$0,12
	sw	$3,180($30)
	jal	.Lsplit_68b8
	addiu	$10,$0,12
	addiu	$3,$0,48
	sw	$3,180($30)
	jal	.Lsplit_68cc
	addiu	$10,$0,12
	addiu	$3,$0,60
	sw	$3,180($30)
	lw	$31,56($25)
	addiu	$25,$25,-4
	bgez	$0,.Lsplit_68e0
	addiu	$10,$0,12
.Lsplit_6ae8:
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
.Lsplit_6bf4:
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lsplit_689c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lsplit_6cec
	addiu	$3,$0,12
	sw	$10,112($1)
	lw	$8,108($1)
	lw	$9,124($1)
	lw	$10,404($1)
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lsplit_6c8c
	neg	$8,$8
	xor	$8,$8,$10
.Lsplit_6c8c:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lsplit_689c
	sll	$15,$15,0x10
	bltz	$15,.Lsplit_689c
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lsplit_6ccc
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lsplit_6ccc:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	lw	$10,112($1)
	or	$3,$3,$8
	and	$3,$3,$10
	beq	$3,$0,.Lsplit_689c
.Lsplit_6cec:
	sw	$3,112($1)
	sw	$31,60($25)
	addiu	$8,$1,804
	beq	$8,$30,.Lsplit_70a4
	addiu	$25,$25,4
	lhu	$11,16($19)
	lhu	$12,16($20)
	lhu	$13,16($21)
	lhu	$14,16($22)
	addiu	$17,$11,-320
	addiu	$18,$12,-320
	addiu	$15,$13,-320
	bgez	$0,.Lsplit_6fc0
	addiu	$16,$14,-320
.Lsplit_6d24:
	DRAW_LEVEL_OVR_DECODE_QUAD_VERTEX_POINTERS
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lsplit_689c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lsplit_6e74
	addiu	$3,$0,12
	DRAW_LEVEL_OVR_BEGIN_QUAD_CLIP_TEST
	beq	$8,$0,.Lsplit_6e1c
	neg	$8,$8
	xor	$8,$8,$10
.Lsplit_6e1c:
	c2	0x1400006
	and	$3,$11,$12
	and	$3,$3,$13
	and	$3,$3,$14
	or	$15,$15,$16
	or	$15,$15,$17
	nor	$15,$15,$18
	or	$15,$15,$3
	bltz	$15,.Lsplit_689c
	sll	$15,$15,0x10
	bltz	$15,.Lsplit_689c
	mfc2	$3,$24
	or	$8,$8,$9
	beq	$3,$0,.Lsplit_6e5c
	srl	$8,$8,0x1f
	xor	$3,$3,$10
.Lsplit_6e5c:
	sll	$8,$8,0x2
	or	$3,$3,$9
	srl	$3,$3,0x1f
	sll	$3,$3,0x3
	or	$3,$3,$8
	beq	$3,$0,.Lsplit_689c
.Lsplit_6e74:
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
	bgtz	$10,.Lsplit_6ec8
	addu	$28,$11,$0
	addu	$28,$12,$0
.Lsplit_6ec8:
	subu	$10,$28,$13
	bgtz	$10,.Lsplit_6ed8
	addu	$3,$3,$24
	addu	$28,$13,$0
.Lsplit_6ed8:
	subu	$10,$28,$14
	bgtz	$10,.Lsplit_6ee8
	lw	$3,28($3)
	addu	$28,$14,$0
.Lsplit_6ee8:
	addu	$10,$28,$0
	srl	$28,$28,0x6
	addu	$28,$28,$9
	andi	$9,$3,0x1
	beq	$9,$0,.Lsplit_6f04
	sll	$28,$28,0x2
	lw	$3,-1($3)
.Lsplit_6f04:
	bgez	$28,.Lsplit_6f10
	addu	$28,$28,$5
	addu	$28,$5,$0
.Lsplit_6f10:
	lw	$9,36($3)
	addiu	$8,$10,-1600
	sw	$9,132($1)
	bgez	$8,.Lsplit_6f48
	addiu	$8,$10,-1280
	addiu	$3,$3,12
	bgez	$8,.Lsplit_6f48
	addiu	$8,$10,-640
	addiu	$3,$3,12
	bgez	$8,.Lsplit_6f48
	sll	$0,$0,0x0
	bltz	$9,.Lsplit_6f48
	sll	$0,$0,0x0
	addiu	$3,$3,12
.Lsplit_6f48:
	lw	$10,404($1)
	lw	$8,0($3)
	lw	$9,4($3)
	sw	$8,416($1)
	sw	$8,428($1)
	sw	$9,420($1)
	sll	$10,$10,0x8
	bltz	$10,.Lsplit_6f90
	sw	$9,432($1)
	sh	$8,6($19)
	lw	$8,8($3)
	sh	$9,6($20)
	sh	$8,6($21)
	sll	$10,$10,0x10
	bltz	$10,.Lsplit_6fb0
	srl	$9,$8,0x10
	bgez	$0,.Lsplit_6fb0
	sh	$9,6($22)
.Lsplit_6f90:
	sh	$9,6($19)
	lw	$9,8($3)
	sll	$10,$10,0x10
	sh	$8,6($20)
	srl	$8,$9,0x10
	bltz	$10,.Lsplit_6fb0
	sh	$8,6($21)
	sh	$9,6($22)
.Lsplit_6fb0:
	addiu	$17,$11,-640
	addiu	$18,$12,-640
	addiu	$15,$13,-640
	addiu	$16,$14,-640
.Lsplit_6fc0:
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
	beq	$17,$0,.Lsplit_7088
	addu	$13,$19,$0
	addu	$14,$20,$0
	addiu	$30,$30,184
	addiu	$15,$30,0
	jal	.Lsplit_6ae8
	addiu	$16,$30,80
	addu	$13,$20,$0
	addu	$14,$22,$0
	addiu	$15,$30,20
	jal	.Lsplit_6ae8
	addiu	$16,$30,140
	addu	$13,$21,$0
	addu	$14,$19,$0
	addiu	$15,$30,40
	jal	.Lsplit_6ae8
	addiu	$16,$30,100
	addu	$13,$22,$0
	addu	$14,$21,$0
	addiu	$15,$30,60
	jal	.Lsplit_6ae8
	addiu	$16,$30,160
	addu	$13,$20,$0
	addu	$14,$21,$0
	addiu	$15,$30,20
	jal	.Lsplit_6ae8
	addiu	$16,$30,120
	addu	$3,$17,$1
	lw	$3,328($3)
	lui	$8,%hi(.Lsplit_7078)
	addiu	$8,$8,%lo(.Lsplit_7078)
	sw	$3,156($1)
	sw	$8,60($25)
	jr	$3
	addiu	$25,$25,4
.Lsplit_7078:
	lw	$31,56($25)
	addiu	$25,$25,-4
	jr	$31
	addiu	$30,$30,-184
.Lsplit_7088:
	lw	$3,112($1)
	addiu	$25,$25,-4
	addu	$3,$3,$1
	lw	$3,388($3)
	lw	$31,60($25)
	jr	$3
	sll	$0,$0,0x0
.Lsplit_70a4:
	lw	$3,132($1)
	addiu	$10,$1,804
	bgtz	$3,.Lsplit_7148
	lw	$8,800($1)
	lw	$9,984($1)
	bne	$10,$30,.Lsplit_7114
	lw	$3,156($1)
	lui	$10,%hi(.Lsplit_6aa4)
	addiu	$10,$10,%lo(.Lsplit_6aa4)
	bne	$3,$10,.Lsplit_7148
	lw	$3,404($1)
	sll	$8,$8,0x1
	sll	$3,$3,0x8
	bgez	$3,.Lsplit_70e4
	lw	$10,132($1)
	addiu	$8,$8,192
.Lsplit_70e4:
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
.Lsplit_7114:
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
.Lsplit_7148:
	lw	$8,428($1)
	lw	$9,432($1)
	sw	$8,416($1)
	bgez	$0,.Lsplit_7114
	sw	$9,420($1)
	addu	$19,$20,$0
	addu	$20,$22,$0
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	and	$3,$15,$16
	and	$3,$3,$17
	bltz	$3,.Lsplit_689c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	or	$15,$15,$16
	or	$15,$15,$17
	bltz	$15,.Lsplit_731c
	DRAW_LEVEL_OVR_LOAD_TRIANGLE_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lsplit_71e4
	lui	$3,0x3600
	lui	$3,0x3400
.Lsplit_71e4:
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
	bgez	$0,.Lsplit_72f8
	lui	$10,0x900
	lb	$15,19($19)
	lb	$16,19($20)
	lb	$17,19($21)
	lb	$18,19($22)
	and	$3,$15,$16
	and	$3,$3,$17
	and	$3,$3,$18
	bne	$3,$0,.Lsplit_689c
	lb	$15,18($19)
	lb	$16,18($20)
	lb	$17,18($21)
	lb	$18,18($22)
	or	$15,$15,$16
	or	$15,$15,$17
	or	$15,$15,$18
	bne	$15,$0,.Lsplit_73c0
	DRAW_LEVEL_OVR_LOAD_QUAD_PACKET_INPUTS
	lui	$3,0x60
	and	$9,$20,$3
	bne	$9,$3,.Lsplit_72b8
	lui	$3,0x3e00
	lui	$3,0x3c00
.Lsplit_72b8:
	DRAW_LEVEL_OVR_WRITE_GT4_PACKET
.Lsplit_72f8:
	lw	$3,0($28)
	sll	$8,$2,0x8
	srl	$8,$8,0x8
	or	$3,$3,$10
	sw	$3,0($2)
	addu	$2,$2,$9
	sw	$8,0($28)
	jr	$31
	addi	$29,$29,1
.Lsplit_731c:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lsplit_7488
	lh	$9,4($19)
	bltz	$10,.Lsplit_736c
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lsplit_7488
	lh	$9,4($20)
	bltz	$10,.Lsplit_736c
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lsplit_7488
	lh	$9,4($21)
	lw	$31,88($1)
	bgez	$10,.Lsplit_689c
.Lsplit_736c:
	lw	$15,16($1)
	lw	$3,128($1)
	lh	$8,422($1)
	lh	$9,418($1)
	sw	$28,4($15)
	sw	$3,0($15)
	sh	$8,8($15)
	sh	$9,10($15)
	addu	$13,$19,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,36
	lw	$31,88($1)
	addiu	$15,$15,48
	jr	$31
	sw	$15,16($1)
.Lsplit_73c0:
	sw	$31,88($1)
	lh	$15,408($1)
	lh	$16,410($1)
	lh	$17,412($1)
	lh	$3,0($19)
	lh	$8,2($19)
	jal	.Lsplit_7488
	lh	$9,4($19)
	bltz	$10,.Lsplit_7424
	lh	$3,0($20)
	lh	$8,2($20)
	jal	.Lsplit_7488
	lh	$9,4($20)
	bltz	$10,.Lsplit_7424
	lh	$3,0($21)
	lh	$8,2($21)
	jal	.Lsplit_7488
	lh	$9,4($21)
	bltz	$10,.Lsplit_7424
	lh	$3,0($22)
	lh	$8,2($22)
	jal	.Lsplit_7488
	lh	$9,4($22)
	lw	$31,88($1)
	bgez	$10,.Lsplit_689c
.Lsplit_7424:
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
	jal	.Lsplit_74c8
	addiu	$16,$15,12
	addu	$13,$20,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,24
	addu	$13,$21,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,36
	addu	$13,$22,$0
	jal	.Lsplit_74c8
	addiu	$16,$15,48
	lw	$31,88($1)
	addiu	$15,$15,60
	jr	$31
	sw	$15,16($1)
.Lsplit_7488:
	subu	$3,$3,$15
	bgez	$3,.Lsplit_7498
	subu	$8,$8,$16
	neg	$3,$3
.Lsplit_7498:
	addiu	$11,$3,-256
	bgez	$8,.Lsplit_74a8
	subu	$9,$9,$17
	neg	$8,$8
.Lsplit_74a8:
	addiu	$12,$8,-384
	bgez	$9,.Lsplit_74b8
	sll	$0,$0,0x0
	neg	$9,$9
.Lsplit_74b8:
	addiu	$13,$9,-256
	and	$11,$11,$12
	jr	$31
	and	$10,$11,$13
.Lsplit_74c8:
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
.Lsplit_74f0:
	lw	$31,672($1)
	sll	$0,$0,0x0
.Lsplit_74f8:
	jr	$31
	sll	$0,$0,0x0
.Lsplit_7500:
	sw	$31,672($1)
	beq	$23,$24,.Lsplit_74f8
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
.Lsplit_75b8:
	beq	$23,$24,.Lsplit_74f0
	lw	$3,48($1)
	addiu	$8,$2,.Lsplit_final_prim_limit
	subu	$3,$3,$8
	bltz	$3,.Lsplit_02bc
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
	DRAW_LEVEL_OVR_SPLIT_CLIP_DISPATCHES \rdata
.end \symbol
.size \symbol, . - \symbol
.endm
