extends Node2D




func _physics_process(delta: float) -> void:
	var space_state = get_world_2d().direct_space_state
	# use global coordinates, not local to node
	var query = PhysicsRayQueryParameters2D.create(global_position, global_position+(Vector2.UP.rotated(global_rotation))*1000)
	var result = space_state.intersect_ray(query)
	
	
	if result:
		#print(result.position-global_position)
		#Events.particle_destroyed.emit(result.position)
		Events.particle_destroyed.emit(result.position-global_position)
		
	
