using System.Collections.Generic;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>EF Core context over every world table. Declares no table itself: each generated record
/// carries its own <see cref="ITableConfiguration"/> and DbSet.</summary>
public sealed partial class WorldDbContext(DbContextOptions<WorldDbContext> options, IReadOnlyList<ITableConfiguration> tableConfigurations)
    : DbContext(options)
{
    protected override void OnModelCreating(ModelBuilder model)
    {
        foreach (ITableConfiguration configuration in tableConfigurations)
        {
            configuration.Configure(model);
        }
    }
}
