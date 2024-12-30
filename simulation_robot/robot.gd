extends CharacterBody2D


const SPEED = 100.0
const TURN_SPEED = SPEED*PI/300.0
const TEST = false
const T_threshold=0.0625*250.0/SPEED

var direction=Vector2.UP

var d_right=0
var d_left=0

var colision_size = 40

var light_on_right=0

@export var state = 0

var initial_rotation=0

var dist_lr=[-2000.0,2000.0]
var min_dist= [2000.0,0]

var marge_erreur_capteur = 0.0
var marge_erreur_mvt = 0.0

var perceived_rotation=0

var first_turn=true
var turn=0.0
var time1=0.0


var threshold_close=54#/cos($capteur_gauche.rotation)
var threshold_wall=90#/cos($capteur_gauche.rotation)
var threshold_no_wall =160#/cos($capteur_gauche.rotation)



func _ready() -> void:
	Events.particle_destroyed.connect(add_turn)
	$capteur_gauche.rotation_degrees=-47
	$capteur_droit.rotation_degrees=47
	#threshold_close=50#/cos($capteur_gauche.rotation)
	#threshold_wall =90#/cos($capteur_gauche.rotation)
	#threshold_no_wall =160#/cos($capteur_gauche.rotation)
	
	match state:
		0:
			#up_direction=Vector2.UP.rotated(randf_range(-PI/2,PI/2))
			up_direction=Vector2.UP.rotated(PI/2)
			
			
		1:
			up_direction=Vector2.UP
		2:
			up_direction=Vector2.UP
		3:
			
			up_direction=Vector2.RIGHT
		4:
			up_direction=Vector2.RIGHT
		5:
			up_direction=Vector2.UP
		6:
			up_direction=Vector2.UP
		7:
			up_direction=Vector2.LEFT
			
			
		8:
			up_direction=Vector2.LEFT
		
	rotation=Vector2.UP.angle_to(up_direction)
	direction=up_direction
	print(up_direction)
	print(rotation)


func _process(delta: float) -> void:
	
	
	#turn(d_right - d_left)
	
	
	#var turn=sign(right_wall_dist)*TURN_SPEED
	if state!=0:
		turn=-min_dist[1]*TURN_SPEED*delta
	
	match state:
		0:
			if initial_rotation==0:
				initial_rotation=sign(rotation)
			if sign(rotation)==initial_rotation and first_turn:
				turn=-sign(rotation)*5*TURN_SPEED*delta
				print("ha")
			else:
				first_turn=false
				turn=-sign(rotation)*0.2*TURN_SPEED*delta
			
			if condition_3_L() and condition_3_R() :
				state+=1
				time1 = 0.0
				perceived_rotation=0
				#get_tree().set_pause(true)
			
			
			
		1:
			
			time1+=delta
			if time1<=75.0/SPEED:
				turn=0
			else:
				state+=1
				time1 = 0.0
				perceived_rotation=0
				
		2:
			turn=1*TURN_SPEED*delta
			if condition_4_R():
				turn=-turn
				print("WALL")
			
			time1+=delta
			perceived_rotation+=turn
			if perceived_rotation>=PI/3:
				print("aaaaarghh")
				if time1>50.0/SPEED:
					state+=1
					time1 = 0.0
		3:
			
			time1+=delta
			if condition_2_L():
				if time1>T_threshold:
					state+=1
					time1 = 0.0
					perceived_rotation=0
		4:
			
			
			turn=-0.9*TURN_SPEED*delta
			if condition_4_R():
				turn=-turn
				print("WALL")
			
			perceived_rotation+=turn
			
			time1+=delta
			if perceived_rotation<=-PI/3:
				perceived_rotation=0
				state+=2
			#if condition_3_R():
				#
				#if time1>25.0/SPEED:
					#state+=1
					#time1 = 0.0
		5:
			turn=-1*TURN_SPEED*delta/2
			if condition_4():
				turn=-turn
				print("WALL")
			
			
			time1+=delta
			if condition_2_R():
				if time1>T_threshold:
					state+=1
					time1 = 0.0
		6:
			turn=-1.25*TURN_SPEED*delta
			if condition_4_L():
				turn=-turn
				print("WALL")
			
			perceived_rotation+=turn
			
			time1+=delta
			print("6 left")
			if perceived_rotation<-PI/2:
				state+=1
				#if time1>25.0/SPEED:
					#state+=1
					#time1 = 0.0
		7:
			time1+=delta
			print("7 straight")
			if condition_2_R() :
				
				if time1>50.0/SPEED:
					state+=1
					time1 = 0.0
					perceived_rotation=0
		8:
			#get_tree().set_pause(true)
			turn=0.9*TURN_SPEED*delta
			if condition_4_R():
				turn=-turn
				print("WALL")
			
			perceived_rotation+=turn
			
			time1+=delta
			if perceived_rotation>=PI/2:
				
				state+=1
				#if time1>50.0/SPEED:
					#state+=1
					#time1 = 0.0
		9:
			pass
		_:
			
			time1+=delta
			if condition_2_R() or condition_2_L():
				if time1>T_threshold:
					state+=1
					time1 = 0.0
			else:
				time1=0.0
				
	var random_factor = randf_range(0.9,1.1)
	rotate(turn*random_factor)
	direction=direction.rotated(turn*random_factor)
	
	random_factor = randf_range(1.0-marge_erreur_mvt,1.0+marge_erreur_mvt)
	velocity = direction * SPEED*random_factor
	if first_turn:
		velocity=Vector2.ZERO
	
	if dist_lr[0]>-threshold_close and state not in [4,6]:
		velocity/=3#halve the speed of the right wheel
		
	elif dist_lr[1]<threshold_close and state not in [2,8]:
		velocity/=3#halve the speed of the left wheel
		#print("GO LEFT")
	
	if move_and_slide():
		$Sprite2D.modulate=Color.RED
		#print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
	
	
	
	#print("min_dist ="+str(min_dist[0]*min_dist[1]))
	#print(state)
	#print(dist_lr)
	light_on_right=0
	min_dist[0]=500.0
	min_dist[1]=0.0
	dist_lr=[-2000.0,2000.0]


func add_turn(global_particle_position:Vector2)->void:
	#var relative_collision=global_particle_position-global_position
	#var dist = (relative_collision).dot(-direction.orthogonal())*randf_range(1.0-marge_erreur_capteur,1.0+marge_erreur_capteur)
	var dist = (global_particle_position).dot(-direction.orthogonal())*randf_range(1.0-marge_erreur_capteur,1.0+marge_erreur_capteur)
	#
	#if dist < abs(300):
	
	dist_lr[(sign(dist)+1)/2]=dist
	if min_dist[0]>abs(dist):
		min_dist=[abs(dist),sign(dist)]



func condition_1()->bool:
	return abs(light_on_right)>170

func condition_2_R()->bool:
	#return dist_lr[1]-dist_lr[0]>280
	return (dist_lr[1]>threshold_no_wall)
func condition_2_L()->bool:
	#return dist_lr[1]-dist_lr[0]>280
	return abs(dist_lr[0])>threshold_no_wall
	
func condition_3_R()->bool:
	#return dist_lr[1]-dist_lr[0]<180
	return abs(dist_lr[1]) < threshold_wall

func condition_3_L()->bool:
	#return dist_lr[1]-dist_lr[0]<180
	#print(max(abs(dist_lr[0]),dist_lr[1]))
	return abs(dist_lr[1]) < threshold_wall


func condition_4()->bool:
	#print(min_dist[0])
	return min_dist[0]<threshold_close

func condition_4_R()->bool:
	#return dist_lr[1]-dist_lr[0]<180
	return abs(dist_lr[1]) < threshold_close

func condition_4_L()->bool:
	#return dist_lr[1]-dist_lr[0]<180
	return abs(dist_lr[0]) < threshold_close

func _on_middle_door_body_entered(body: Node2D) -> void:
	print(state)
	#print(body.get_meta("door_number"))
	#if state >= body.get_meta("door_number"):
		#print("20 copilots")
